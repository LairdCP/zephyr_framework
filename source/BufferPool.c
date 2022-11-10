/**
 * @file BufferPool.c
 * @brief
 *
 * Copyright (c) 2021-2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define FWK_FNAME "BufferPool"
#include <logging/log.h>
LOG_MODULE_REGISTER(buffer_pool, LOG_LEVEL_WRN);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <string.h>

#include "Framework.h"
#include "BufferPool.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
struct bph {
#ifdef CONFIG_BUFFER_POOL_CHECK_DOUBLE_FREE
	void *ptr;
#endif
	uint16_t size;
	uint8_t pool;
	uint8_t reserved;
} __packed;

#define BPH_SIZE sizeof(struct bph)

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static K_HEAP_DEFINE(buffer_pool, CONFIG_BUFFER_POOL_SIZE);

static atomic_t take_failed = ATOMIC_INIT(0);

#ifdef CONFIG_BUFFER_POOL_STATS
static struct bp_stats bps;
#endif

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
#ifdef CONFIG_BUFFER_POOL_STATS
static void TakeStatHandler(struct bph *bph, size_t size);
static void TakeFailStatHandler(size_t size);
static void GiveStatHandler(struct bph *bph);
#endif

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void BufferPool_Initialize(void)
{
#ifdef CONFIG_BUFFER_POOL_STATS
	if (!bps.initialized) {
		bps.initialized = true;
		bps.space_available = CONFIG_BUFFER_POOL_SIZE;
		bps.min_space_available = CONFIG_BUFFER_POOL_SIZE;
		bps.min_size = CONFIG_BUFFER_POOL_SIZE;
	}
#endif
}

void *BufferPool_TryToTakeTimeout(size_t size, k_timeout_t timeout,
				  const char *const context)
{
	size_t size_with_header = size + BPH_SIZE;
	uint8_t *p = k_heap_alloc(&buffer_pool, size_with_header, timeout);

	if (p != NULL) {
		memset(p, 0, size_with_header);
#ifdef CONFIG_BUFFER_POOL_STATS
		TakeStatHandler((struct bph *)p, size);
#endif
		return p + BPH_SIZE;
	} else {
		/* A timeout can occur even when there is space available. */
		LOG_WRN("Allocate failure size: %d context: %s", size, context);
#ifdef CONFIG_BUFFER_POOL_STATS
		TakeFailStatHandler(size);
#endif
		return p;
	}
}

void *BufferPool_TryToTake(size_t size, const char *const context)
{
	return BufferPool_TryToTakeTimeout(size, K_NO_WAIT, context);
}

void *BufferPool_Take(size_t size)
{
	void *ptr = BufferPool_TryToTake(size, BP_CONTEXT_UNUSED);

	if (ptr == NULL) {
		/* Prevent recursive entry. */
		if (atomic_get(&take_failed) == 0) {
			atomic_set(&take_failed, 1);
			LOG_ERR("Buffer pool too small");
			FRAMEWORK_ASSERT(FORCED);
		}
	}
	return ptr;
}

void BufferPool_Free(void *pBuffer)
{
	uint8_t *p = pBuffer;

	if (p == NULL) {
		LOG_ERR("Attempt to free NULL buffer");
		return;
	}

	p -= BPH_SIZE;

#ifdef CONFIG_BUFFER_POOL_STATS
	GiveStatHandler((struct bph *)p);
#endif

	k_heap_free(&buffer_pool, p);
}

int BufferPool_GetStats(uint8_t index, struct bp_stats *stats)
{
#ifdef CONFIG_BUFFER_POOL_STATS
	if (index == 0 && stats != NULL) {
		k_spinlock_key_t key = k_spin_lock(&buffer_pool.lock);
		memcpy(stats, &bps, sizeof(struct bp_stats));
		k_spin_unlock(&buffer_pool.lock, key);
		return 0;
	}
#endif

	return -EINVAL;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
#ifdef CONFIG_BUFFER_POOL_STATS
static void TakeStatHandler(struct bph *bph, size_t size)
{
	k_spinlock_key_t key = k_spin_lock(&buffer_pool.lock);

	bph->size = size;
#ifdef CONFIG_BUFFER_POOL_CHECK_DOUBLE_FREE
	bph->ptr = bph;
#endif

	bps.space_available -= size;
	bps.min_space_available =
		MIN(bps.min_space_available, bps.space_available);
	bps.min_size = MIN(bps.min_size, size);
	bps.max_size = MAX(bps.max_size, size);
	bps.allocs += 1;
	bps.cur_allocs += 1;
	bps.max_allocs = MAX(bps.max_allocs, bps.cur_allocs);
#if CONFIG_BUFFER_POOL_WINDOW_SIZE > 0
	bps.window[bps.windex++] = size;
	if (bps.windex >= CONFIG_BUFFER_POOL_WINDOW_SIZE) {
		bps.windex = 0;
	}
#endif

	k_spin_unlock(&buffer_pool.lock, key);
}

static void TakeFailStatHandler(size_t size)
{
	k_spinlock_key_t key = k_spin_lock(&buffer_pool.lock);

	bps.take_failures += 1;
	bps.last_fail_size = size;
	bps.min_space_available =
		MIN(bps.min_space_available, (bps.space_available - size));

	k_spin_unlock(&buffer_pool.lock, key);
}

static void GiveStatHandler(struct bph *bph)
{
	k_spinlock_key_t key = k_spin_lock(&buffer_pool.lock);

#ifdef CONFIG_BUFFER_POOL_CHECK_DOUBLE_FREE
	if (bph->ptr == 0) {
		LOG_ERR("Buffer Pool Possible Duplicate Free");
	} else if (bph->ptr == bph) {
		/* This is going back into buffer pool.
		 * It will either be assigned a new value or remain at 0.
		 * A breakpoint on this location may point to the wrong offender,
		 * but still indicates a double free condition.
		 */
		bph->ptr = 0;
	} else {
		LOG_ERR("Buffer Pool Free Error");
	}
#endif

	bps.space_available += bph->size;
	bps.cur_allocs -= 1;

	k_spin_unlock(&buffer_pool.lock, key);
}
#endif
