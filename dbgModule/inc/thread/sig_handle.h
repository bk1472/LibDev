/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file signal.h
 *
 *	signal hnadler libraries API export
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 4. 10
 *	@note
 *	@see
 */
#ifndef __SIG_HANDLE_H__
#define __SIG_HANDLE_H__
/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"
#include	<signal.h>
#include	<unistd.h>
#include	<ucontext.h>

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef void (*SIG_HADLER_T) (int, siginfo_t*, void*);
typedef void (*SIG_INSTAL_T) (int);

typedef enum
{
	SIG_EXCEPT,
	SIG_KILL,
	SIG_CMD,
	SIG_MAX
} sig_type_t;

/*-----------------------------------------------------------------------------
 * External Used variables
------------------------------------------------------------------------------*/
extern int		register_signal			(int sigNo, SIG_HADLER_T p_handler);
extern int		unregister_signal		(int sigNo);
extern char* 	getSignalName			(int sigNo);
extern void		get_signal_backtrace	(void* data);

#ifdef __cplusplus
}
#endif

#endif/*__SIG_HANDLE_H__*/
