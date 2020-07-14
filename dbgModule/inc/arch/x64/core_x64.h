/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file core_x64.h
 *
 *	X64 CPU core dependent types and definitions
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 4. 10
 *	@note
 *	@see
 */
#ifndef __CORE_X64_H__
#define __CORE_X64_H__

/*-----------------------------------------------------------------------------
 * Include Headers
------------------------------------------------------------------------------*/
#include	<stdint.h>
#include	<ucontext.h>

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Constant Constants
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef enum
{
	X64_R08 = 0,
#define X64_R08		X64_08
	X64_R09 = 1,
#define X64_R09     X64_R09
	X64_R10 =2,
#define X64_R10     X64_R10
	X64_R11 = 3,
#define X64_R11     X64_R11
	X64_R12 = 4,
#define X64_R12     X64_R12
	X64_R13 = 5,
#define X64_R13     X64_R13
	X64_R14 = 6,
#define X64_R14     X64_R14
	X64_R15 = 7,
#define X64_R15     X64_R15
	X64_RDI = 8,
#define X64_RDI     X64_RDI
	X64_RSI = 9,
#define X64_RSI     X64_RSI
	X64_RBP = 10,
#define X64_RBP     X64_RBP
	X64_RBX = 11,
#define X64_RBX     X64_RBX
	X64_RDX = 12,
#define X64_RDX     X64_RDX
	X64_RAX = 13,
#define X64_RAX     X64_RAX
	X64_RCX = 14,
#define X64_RCX     X64_RCX
	X64_RSP = 15,
#define X64_RSP     X64_RSP
	X64_RIP = 16,
#define X64_RIP     X64_RIP
	X64_EFLAGS = 17,
#define X64_ELFAGS  X64_ELFAGS
	X64_CS = 18,
#define X64_CS      X64_CS
	X64_GS = 19,
#define X64_GS      X64_GS
	X64_FS = 20,
#define X64_FS      X64_FS
	X64_PAD0 = 21,
#define X64_PAD0    X64_PAD0
	X64_ERR = 22,
#define X64_ERR     X64_ERR
	X64_TRAPNO = 23,
#define X64_TRAPNO  X64_TRAPNO
	X64_OLDMSK = 24,
#define X64_OLDMSK  X64_OLDMSK
	X64_CR2 = 25,
#define X64_CR2     X64_CR2
	X64_MAX = 26
} reg_idx_t;

typedef struct
{
	uint64_t	reg[X64_MAX];
} context_t;


/*-----------------------------------------------------------------------------
 * External Used variables
------------------------------------------------------------------------------*/
extern uint64_t*	x64_get_pc				(void);
extern uint64_t*	x64_get_ra				(void);

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/
#define	str_sp(_x)	do { __asm__ ("movq %%rsp, (%0)" : /**/ : "r" (_x)); } while(0)
#define	str_ra(_x)	do { *((uint64_t**)_x) = (uint64_t*)__builtin_frame_address(0); \
						*((uint64_t**)_x) = (uint64_t*)((*_x))[1];  } while(0)
#define	str_pc(_x)	do { *((uint64_t**)_x) = (uint64_t*)x64_get_pc();    } while(0)

#ifdef __cplusplus
}
#endif

#endif/*__CORE_X64_H__*/
