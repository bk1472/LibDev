/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file context_aarch64.c
 *
 *	aarch64 dependenct code
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 4. 10
 *	@note
 *	@see
 */

/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"
#include	"xlibc.h"
#include	"thread.h"
#include	"symbol.h"
#include	"arch.h"
#include	"dsm.h"
#include	"backtrace.h"


/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Static Variables & Function prototypes declarations
------------------------------------------------------------------------------*/
static context_t	_ctxt;

static reg_info_t	_reginfo[ARM64_MAX+1] =
{
	{"faddr",  &_ctxt.reg[ 0]},	{"r0",     &_ctxt.reg[ 1]},
	{"r1",     &_ctxt.reg[ 2]},	{"r2",     &_ctxt.reg[ 3]},
	{"r3",     &_ctxt.reg[ 4]},	{"r4",     &_ctxt.reg[ 5]},
	{"r5",     &_ctxt.reg[ 6]},	{"r6",     &_ctxt.reg[ 7]},
	{"r7",     &_ctxt.reg[ 8]},	{"r8",     &_ctxt.reg[ 9]},
	{"r9",     &_ctxt.reg[10]},	{"r10",    &_ctxt.reg[11]},
	{"r11",    &_ctxt.reg[12]},	{"r12",    &_ctxt.reg[13]},
	{"r13",    &_ctxt.reg[14]},	{"r14",    &_ctxt.reg[15]},
	{"r15",    &_ctxt.reg[16]},	{"r16",    &_ctxt.reg[17]},
	{"r17",    &_ctxt.reg[18]},	{"r18",    &_ctxt.reg[19]},
	{"r19",    &_ctxt.reg[20]},	{"r20",    &_ctxt.reg[21]},
	{"r21",    &_ctxt.reg[22]},	{"r22",    &_ctxt.reg[23]},
	{"r23",    &_ctxt.reg[24]},	{"r24",    &_ctxt.reg[25]},
	{"r25",    &_ctxt.reg[26]},	{"r26",    &_ctxt.reg[27]},
	{"r27",    &_ctxt.reg[28]},	{"r28",    &_ctxt.reg[29]},
	{"r29",    &_ctxt.reg[30]},	{"r30",    &_ctxt.reg[31]},
	{"sp",     &_ctxt.reg[32]},	{"pc",     &_ctxt.reg[33]},
	{"pstate", &_ctxt.reg[34]},	{NULL}
};


/*-----------------------------------------------------------------------------
 *	Static Function definitions
------------------------------------------------------------------------------*/
static void _print_dsm_exc(uint64_t pc, int window_sz, dsm_prnt_t pFunc)
{
	uint64_t	dsmPc, symPc;
	char*		symName;
	int			i;

	if(isValidPc(pc) == false)
		return;

	symPc = findSymByAddr(pc, &symName);

	if(symPc)
	{
		dsmPc = pc - (window_sz*2);
		while(symPc < dsmPc)
		{
			symPc += dsmInst((inst_t*)symPc, (uint64_t)symPc, (void*)pFunc);
		}
		dsmPc = symPc;
	}
	else
	{
		dsmPc = pc;
	}

	(*pFunc)("\n        [SYMBOL : %-30s]\n\n", symName);

	for(i = 0; i < window_sz; i++)
	{
		if(dsmPc == pc) (*pFunc)("     ** ");
		else            (*pFunc)("        ");
		dsmPc += dsmInst((inst_t*)dsmPc, (uint64_t)dsmPc, pFunc);
	}
	(*pFunc)("\n");
}

static void _print_callstack(printf_proc_t pFunc)
{
	frame_reg_t	frame;

	frame.pc = REG(ARM64_PC);
	frame.sp = REG(ARM64_SP);
	frame.ra = REG(ARM64_PC);

	if( isValidPc(frame.pc) == false
	 || isValidAddr(frame.sp) == false)
		return;

	if(pFunc)
		stack_trace(0x800, 32, NULL, &frame, pFunc);
}

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
frame_reg_t get_frame_sig(void)
{
	frame_reg_t	frame;

	#if 0
	_print_reg_map(rprint0n);
	#endif
	frame.pc = REG(ARM64_PC);
	frame.sp = REG(ARM64_SP);
	frame.ra = REG(ARM64_PC);
	/**
	 * AARCH64 의 경우 user signal 에 의해서
	 * 받은 정보는 svc system call의 argument를 직접 넣어준다.
	 */
	frame.pc = ((uint64_t*)frame.sp)[9];
	frame.sp = ((uint64_t*)frame.sp)[8];

	return frame;
}

#include	<sys/ioctl.h>
frame_reg_t get_frame_drv(pid_t pid)
{
	frame_reg_t	frame;
	int			fd;

	frame.pid = pid;
	frame.pc  = 0;
	frame.sp  = 0;
	frame.ra  = 0;

	if ( (fd = get_drv_fd()) < 0)
		return frame;

	ioctl(fd, LG_READ_CONTEXT, &frame);

	frame.pc = ((uint64_t*)frame.sp)[9];
	frame.sp = ((uint64_t*)frame.sp)[8];

	return frame;
}

void make_registermap(void* p_context)
{
	ucontext_t* p_ucontext = p_context;
	int			i;
	uint64_t	*mcContext = (uint64_t*)&p_ucontext->uc_mcontext;

	for(i =0; _reginfo[i].name != NULL; i++)
		*_reginfo[i].reg = mcContext[i];
}

void print_context_info(printf_proc_t pFunc)
{

	(*pFunc)("\n[ Exception Context ]\n\n");
	_print_reg_map(pFunc);

	hexdump("Exception Stack", (void*)REG(ARM64_SP), 0x100);

	(*pFunc)("\n\n");

	_print_dsm_exc(REG(ARM64_PC),16, pFunc);
	_print_callstack(pFunc);
}

char* get_reg_name(int n)
{
	return _reginfo[n].name;
}

uint64_t get_reg_val(int n)
{
	return _reginfo[n].reg[0];
}
