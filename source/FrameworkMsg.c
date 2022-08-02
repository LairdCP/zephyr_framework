/**
 * @file FrameworkMsg.c
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#define FWK_FNAME "FrameworkMsg"

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include "BufferPool.h"
#include "Framework.h"
#include "FrameworkMsg.h"

#ifdef CONFIG_FILTER
#include <framework_ids.h>
#endif

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void DeallocateOnError(FwkMsg_t *pMsg, BaseType_t status);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
BaseType_t FwkMsg_Send(FwkMsg_t *pMsg)
{
	BaseType_t result = Framework_Send(pMsg->header.rxId, pMsg);
	DeallocateOnError(pMsg, result);
	FRAMEWORK_ASSERT(result == FWK_SUCCESS);

	return result;
}

BaseType_t FwkMsg_TryToSend(FwkMsg_t *pMsg)
{
	BaseType_t result = Framework_Send(pMsg->header.rxId, pMsg);
	DeallocateOnError(pMsg, result);

	return result;
}

BaseType_t FwkMsg_SendTo(FwkMsg_t *pMsg, FwkId_t DestId)
{
	pMsg->header.rxId = DestId;
	BaseType_t result = Framework_Send(pMsg->header.rxId, pMsg);
	DeallocateOnError(pMsg, result);
	FRAMEWORK_ASSERT(result == FWK_SUCCESS);

	return result;
}

BaseType_t FwkMsg_Unicast(FwkMsg_t *pMsg)
{
	BaseType_t result = Framework_Unicast(pMsg);
	DeallocateOnError(pMsg, result);
	FRAMEWORK_ASSERT(result == FWK_SUCCESS);

	return result;
}

BaseType_t FwkMsg_CreateAndSend(FwkId_t TxId, FwkId_t RxId, FwkMsgCode_t Code)
{
	BaseType_t result = FWK_ERROR;
	FwkMsg_t *pMsg = (FwkMsg_t *)BufferPool_Take(sizeof(FwkMsg_t));
	FRAMEWORK_ASSERT(pMsg != NULL);

	if (pMsg != NULL) {
		FRAMEWORK_MSG_HEADER_INIT(pMsg, Code, TxId);
		result = Framework_Send(RxId, pMsg);
		DeallocateOnError(pMsg, result);
		FRAMEWORK_ASSERT(result == FWK_SUCCESS);
	}

	return result;
}

BaseType_t FwkMsg_CreateAndSendToSelf(FwkId_t Id, FwkMsgCode_t Code)
{
	BaseType_t result = FWK_ERROR;
	FwkMsg_t *pMsg = (FwkMsg_t *)BufferPool_Take(sizeof(FwkMsg_t));
	FRAMEWORK_ASSERT(pMsg != NULL);

	if (pMsg != NULL) {
		FRAMEWORK_MSG_HEADER_INIT(pMsg, Code, Id);
		pMsg->header.rxId = Id;
		result = Framework_Send(Id, pMsg);
		DeallocateOnError(pMsg, result);
		FRAMEWORK_ASSERT(result == FWK_SUCCESS);
	}

	return result;
}

BaseType_t FwkMsg_UnicastCreateAndSend(FwkId_t TxId, FwkMsgCode_t Code)
{
	BaseType_t result = FWK_ERROR;
	FwkMsg_t *pMsg = (FwkMsg_t *)BufferPool_Take(sizeof(FwkMsg_t));
	FRAMEWORK_ASSERT(pMsg != NULL);

	if (pMsg != NULL) {
		FRAMEWORK_MSG_HEADER_INIT(pMsg, Code, TxId);
		result = Framework_Unicast(pMsg);
		DeallocateOnError(pMsg, result);
		FRAMEWORK_ASSERT(result == FWK_SUCCESS);
	}

	return result;
}

BaseType_t FwkMsg_CreateAndBroadcast(FwkId_t TxId, FwkMsgCode_t Code)
{
	BaseType_t result = FWK_ERROR;
	size_t size = sizeof(FwkMsg_t);
	FwkMsg_t *pMsg = BufferPool_Take(size);

	if (pMsg != NULL) {
		FRAMEWORK_MSG_HEADER_INIT(pMsg, Code, TxId);
		pMsg->header.rxId = FWK_ID_RESERVED;
		result = Framework_Broadcast(pMsg, size);
		DeallocateOnError(pMsg, result);
	}

	return result;
}

BaseType_t FwkMsg_Reply(FwkMsg_t *pMsg, FwkMsgCode_t Code)
{
	BaseType_t result = FWK_ERROR;
	FwkId_t swap = pMsg->header.rxId;
	pMsg->header.rxId = pMsg->header.txId;
	pMsg->header.txId = swap;
	pMsg->header.msgCode = Code;

	result = Framework_Send(pMsg->header.rxId, pMsg);
	DeallocateOnError(pMsg, result);
	FRAMEWORK_ASSERT(result == FWK_SUCCESS);

	return result;
}

BaseType_t FwkMsg_CallbackCreateAndSend(FwkId_t TxId, FwkId_t RxId,
					FwkMsgCode_t Code,
					void (*Callback)(uint32_t),
					uint32_t CallbackData)
{
	BaseType_t result = FWK_ERROR;
	size_t size = sizeof(FwkCallbackMsg_t);
	FwkCallbackMsg_t *pMsg = BufferPool_Take(size);

	if (pMsg != NULL) {
		pMsg->header.msgCode = Code;
		pMsg->header.rxId = RxId;
		pMsg->header.txId = TxId;
		pMsg->header.options = FWK_MSG_OPTION_CALLBACK;
		pMsg->callback = Callback;
		pMsg->data = CallbackData;
		if (RxId == FWK_ID_RESERVED) {
			result = Framework_Unicast((FwkMsg_t *)pMsg);
		} else {
			result = Framework_Send(RxId, (FwkMsg_t *)pMsg);
		}
		DeallocateOnError((FwkMsg_t *)pMsg, result);
	}

	return result;
}

BaseType_t FwkMsg_FilteredTargetedSend(FwkMsg_t *pMsg, FwkId_t *TargetID,
					size_t MsgSize)
{
	BaseType_t result;

	if (TargetID == NULL) {
#ifdef CONFIG_FILTER
		/* With filtering, send targeted message to filter */
		pMsg->header.rxId = FWK_ID_EVENT_FILTER;
		result = Framework_Send(FWK_ID_EVENT_FILTER, pMsg);
#else
		/* Without filtering, send broadcast */
		pMsg->header.rxId = FWK_ID_RESERVED;
		result = Framework_Broadcast(pMsg, MsgSize);
#endif
	} else {
		/* Targeted message, send only to target */
		pMsg->header.rxId = *TargetID;
		result = Framework_Send(*TargetID, pMsg);
	}

	DeallocateOnError(pMsg, result);
	FRAMEWORK_ASSERT(result == FWK_SUCCESS);

	return result;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void DeallocateOnError(FwkMsg_t *pMsg, BaseType_t status)
{
	if (status != FWK_SUCCESS) {
		BufferPool_Free(pMsg);
	}
}
