/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** backtrace.h
 *
 *	stack backtrace libraries
 *
 *	@author			Baekwon Choi (baekwon.choi@lge.com)
 *	@version        1.0
 *	@date           2018. 4. 4
 *	@note
 *	@see
 */
#ifndef __BACKTRACE_H__
#define __BACKTRACE_H__
/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"
#include	"arch.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define STACK_LOG_DEPTH		(6)

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * External Used variables
------------------------------------------------------------------------------*/
/*get_frame.c : architecture dependency  file*/

extern int32_t		get_frame_size		(uint64_t* sp, uint64_t* usl, uint64_t* pc, uint64_t** ra);

/*backtrace.c : architecture independent file*/

extern void			stack_trace			(uint32_t size, uint32_t count, uint64_t *log, void* p_regs, printf_proc_t pFunc);
extern void			log_stack			(uint64_t* log, void* p_regs, int size);
extern void			print_stack			(void);

#ifdef __cplusplus
}
#endif

#endif/*__BACKTRACE_H__*/
