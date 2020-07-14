/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** arch_core.h
 *
 *	architecture dependency definition includes
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 4. 4
 *	@note
 *	@see
 */
#ifndef __ARCH_CORE_H__
#define __ARCH_CORE_H__
/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/
#ifdef	__X64__
#include	"core_x64.h"
#endif

#ifdef	__ARM64__
#include	"core_aarch64.h"
#endif

extern uint64_t	get_reg_val		(int n);
#define REG(_n)		(get_reg_val(_n))
extern char*	get_reg_name	(int n);
#define REG_NM(_n)	(get_reg_name(_n))

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{
	char*		name;
	uint64_t*	reg;
} reg_info_t;

typedef struct
{
	pid_t		pid;	/* thread tid or process pid          */
	uint64_t	pc;     /* program counter (execution pointer */
	uint64_t	ra;     /* return address                     */
	uint64_t	sp;     /* current stack pointer              */
} frame_reg_t;

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/
#define LG_DRV_IOCTL_NUM		('L')
#define LG_READ_CONTEXT			_IOWR(LG_DRV_IOCTL_NUM, 0, frame_reg_t)

/*-----------------------------------------------------------------------------
 * Inline static functions
------------------------------------------------------------------------------*/
static inline void _print_reg_map(printf_proc_t pFunc)
{
	for(int i = 0; REG_NM(i) != NULL; i++)
	{
		(*pFunc)(" %6.6s / 0x%016lx", REG_NM(i), REG(i));
		if(i!=0 && ((i+1)%3) == 0)
			(*pFunc)("\n");
	}
	(*pFunc)("\n\n\n");
}


/*-----------------------------------------------------------------------------
 * External Used variables
------------------------------------------------------------------------------*/
extern frame_reg_t		get_frame_sig				(void);
extern frame_reg_t		get_frame_drv				(pid_t);
extern void				make_registermap			(void* p_context);
extern void				print_context_info			(printf_proc_t pFunc);

extern void				get_thread_backtrace		(void* data, bool drvMod);

#ifdef __cplusplus
}
#endif

#endif/*__ARCH_CORE_H__*/
