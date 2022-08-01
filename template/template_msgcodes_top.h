#ifndef __FRAMEWORK_MSG_CODES_H__
#define __FRAMEWORK_MSG_CODES_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include "Framework.h"

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
enum FwkMsgCodeEnum {
	/* Message codes required by framework */
	FMC_INVALID = 0,
	FMC_PERIODIC,
	FMC_SOFTWARE_RESET,

	/* Application specific */
