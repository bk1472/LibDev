/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file core_aarch64.h
 *
 *	aarch64 CPU core dependent types and definitions
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 4. 10
 *	@note
 *	@see
 */
#ifndef __CORE_AARCH64_H__
#define __CORE_AARCH64_H__

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
	ARM64_FAULT_ADDR = 0,
	ARM64_R0         = 1,
	ARM64_R1         = 2,
	ARM64_R2         = 3,
	ARM64_R3         = 4,
	ARM64_R4         = 5,
	ARM64_R5         = 6,
	ARM64_R6         = 7,
	ARM64_R7         = 8,
	ARM64_R8         = 9,
	ARM64_R9         = 10,
	ARM64_R10        = 11,
	ARM64_R11        = 12,
	ARM64_R12        = 13,
	ARM64_R13        = 14,
	ARM64_R14        = 15,
	ARM64_R15        = 16,
	ARM64_R16        = 17,
	ARM64_R17        = 18,
	ARM64_R18        = 19,
	ARM64_R19        = 20,
	ARM64_R20        = 21,
	ARM64_R21        = 22,
	ARM64_R22        = 23,
	ARM64_R23        = 24,
	ARM64_R24        = 25,
	ARM64_R25        = 26,
	ARM64_R26        = 27,
	ARM64_R27        = 28,
	ARM64_R28        = 29,
	ARM64_R29        = 30,
	ARM64_R30        = 31,
	ARM64_SP         = 32,
	ARM64_PC         = 33,
	ARM64_PSTATE     = 34,
	ARM64_MAX        = 35
} reg_idx_t;

typedef struct
{
	uint64_t	reg[ARM64_MAX];
} context_t;

/*-----------------------------------------------------------------------------
 * External Used variables
------------------------------------------------------------------------------*/
extern uint64_t*	aarch64_get_pc				(void);

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/
#define str_sp(_x)  { __asm__ __volatile__("mov x1, sp; str x1, [%[in]]" : /* no output */ : [in] "r"(_x):"memory"); }
#define str_ra(_x)  { __asm__ __volatile__("str x30, [%[in]]"            : /* no output */ : [in] "r"(_x):"memory"); }
#define	str_pc(_x)	do { *((uint64_t**)_x) = (uint64_t*)aarch64_get_pc();    } while(0)

#ifdef __cplusplus
}
#endif

#endif/*__CORE_AARCH64_H__*/
