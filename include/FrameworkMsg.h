/**
 * @file FrameworkMsg.h
 * @brief Helper/Wrapper functions for sending framework messages.
 *
 * If enabled, functions assert on error.
 * They always deallocate messages on error.
 *
 * @note In FrameworkMacros.h these functions are renamed to be
 * compatible with previous revisions of the framework.
 *
 * Copyright (c) 2020-2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __FRAMEWORK_MSG_H__
#define __FRAMEWORK_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include "Framework.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define FRAMEWORK_MSG_HEADER_INIT(p, c, i)                                     \
	do {                                                                   \
		FRAMEWORK_ASSERT((p) != NULL);                                 \
		p->header.msgCode = (c);                                       \
		p->header.rxId = FWK_ID_RESERVED;                              \
		p->header.txId = (i);                                          \
		p->header.options = FWK_MSG_OPTION_NONE;                       \
	} while (0)

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

/**
 * @brief Wrapper for Framework_Send
 *
 * @param pMsg pointer to a framework message
 *
 * @retval FWK_SUCCESS or FWK_ERROR
 */
BaseType_t FwkMsg_Send(FwkMsg_t *pMsg);

/**
 * @brief Wrapper for Framework_Send that doesn't assert if dest queue is full.
 *
 * @param pMsg pointer to a framework message
 *
 * @retval FWK_SUCCESS or FWK_ERROR
 */
BaseType_t FwkMsg_TryToSend(FwkMsg_t *pMsg);

/**
 * @brief Wrapper for Framework_Send when used with FRAMEWORK_MSG_HEADER_INIT.
 *
 * @param pMsg pointer to a framework message
 * @param DestId is used to set pMsg->header.rxId
 *
 * @retval FWK_SUCCESS or FWK_ERROR
 */
BaseType_t FwkMsg_SendTo(FwkMsg_t *pMsg, FwkId_t DestId);

/**
 * @brief Wrapper for Framework_Unicast.
 *
 * @param pMsg pointer to a framework message
 *
 * @retval FWK_SUCCESS or FWK_ERROR
 */
BaseType_t FwkMsg_Unicast(FwkMsg_t *pMsg);

/**
 * @brief Allocates message from buffer pool and sends it using Framework_Send.
 *
 * @param TxId source of message
 * @param RxId destination of message
 * @param Code message type
 *
 * @retval FWK_SUCCESS or FWK_ERROR
 */
BaseType_t FwkMsg_CreateAndSend(FwkId_t TxId, FwkId_t RxId, FwkMsgCode_t Code);

/**
 * @brief Shorter form of FwkMsg_CreateAndSend that can be used by a task to
 * send a message to itself.
 *
 * @param Id source and destination of message
 * @param Code message type
 *
 * @retval FWK_SUCCESS or FWK_ERROR
 */
BaseType_t FwkMsg_CreateAndSendToSelf(FwkId_t Id, FwkMsgCode_t Code);

/**
 * @brief Allocates message from buffer pool and sends it using
 * Framework_Unicast.
 *
 * @param TxId source of message
 * @param Code message type
 *
 * @retval FWK_SUCCESS or FWK_ERROR
 */
BaseType_t FwkMsg_UnicastCreateAndSend(FwkId_t TxId, FwkMsgCode_t Code);

/**
 * @brief Allocates message from buffer pool and broadcasts it using
 * Framework_Broadcast.
 *
 * @param TxId source of message
 * @param Code message type
 *
 * @retval FWK_SUCCESS or FWK_ERROR
 */
BaseType_t FwkMsg_CreateAndBroadcast(FwkId_t TxId, FwkMsgCode_t Code);

/**
 * @brief Swaps rxId and txId, changes message code, and then sends using
 * Framework_Send.
 *
 * @note Often used with DISPATCH_DO_NOT_FREE.
 * @note The original sender must populate the txId.
 *
 * @param pMsg pointer to a framework message
 * @param Code message type
 *
 * @retval FWK_SUCCESS or FWK_ERROR
 */
BaseType_t FwkMsg_Reply(FwkMsg_t *pMsg, FwkMsgCode_t Code);

/**
 * @brief Allocates callback message from buffer pool and sends it
 *
 * @param TxId source of message
 * @param RxId destination of message
 * @param Code message type
 * If message type is FWK_ID_RESERVED, then unicast is used.
 * @param Callback function called in receiver context
 * @param CallbackData passed into callback function
 *
 * @retval FWK_SUCCESS or FWK_ERROR
 */
BaseType_t FwkMsg_CallbackCreateAndSend(FwkId_t TxId, FwkId_t RxId,
					FwkMsgCode_t Code,
					void (*Callback)(uint32_t),
					uint32_t CallbackData);

/**
 * @brief Sends a targeted framework message to a specific target or if no
 * target is provided, the event filter (or broadcast if event filter is not
 * enabled)
 *
 * @param pMsg pointer to a framework message
 * @param TargetID pointer to target if sending a targeted message, NULL
 * otherwise (for broadcast/filter target)
 * @param MsgSize size (in bytes) of message
 *
 * @retval FWK_SUCCESS or FWK_ERROR
 */
BaseType_t FwkMsg_FilteredTargetedSend(FwkMsg_t *pMsg, FwkId_t *TargetID,
					size_t MsgSize);


#ifdef __cplusplus
}
#endif

#endif /* __FRAMEWORK_MSG_H__ */
