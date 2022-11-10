/**
 * @file FrameworkStubs.c
 * @brief Weak implementations for functions required by framework.
 *
 * Copyright (c) 2020-2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <logging/log.h>
LOG_MODULE_REGISTER(framework_stubs, CONFIG_FRAMEWORK_LOG_LEVEL);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <sys/reboot.h>

#include "Framework.h"

#if defined(CONFIG_LCZ_SOFTWARE_RESET)
#include <lcz_software_reset.h>
#endif

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
__weak void Framework_AssertionHandler(char *file, int line)
{
	UNUSED_PARAMETER(file);
	UNUSED_PARAMETER(line);

	LOG_ERR("assertion: line: %d %s", line, file);

#if defined(CONFIG_LCZ_SOFTWARE_RESET)
	lcz_software_reset_after_assert(0);
#else
	sys_reboot(SYS_REBOOT_WARM);
#endif

}

__weak bool Framework_InterruptContext(void)
{
	return k_is_in_isr();
}

__weak void Framework_SystemReset(void)
{
	return;
}

__weak DispatchResult_t Framework_UnknownMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						    FwkMsg_t *pMsg)
{
	LOG_WRN("Unknown message %u sent to task: %u", pMsg->header.msgCode,
		pMsgRxer->id);

	return DISPATCH_ERROR;
}
