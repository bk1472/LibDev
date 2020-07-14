/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file context_x64.c
 *
 *	x64 dependenct code
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

static reg_info_t	_reginfo[X64_MAX+1] =
{
	{"r8",     &_ctxt.reg[ 0]}, {"r9",     &_ctxt.reg[ 1]},
	{"r10",    &_ctxt.reg[ 2]}, {"r11",    &_ctxt.reg[ 3]},
	{"r12",    &_ctxt.reg[ 4]}, {"r13",    &_ctxt.reg[ 5]},
	{"r14",    &_ctxt.reg[ 6]}, {"r15",    &_ctxt.reg[ 7]},
	{"rdi",    &_ctxt.reg[ 8]}, {"rsi",    &_ctxt.reg[ 9]},
	{"rbp",    &_ctxt.reg[10]}, {"rbx",    &_ctxt.reg[11]},
	{"rdx",    &_ctxt.reg[12]}, {"rax",    &_ctxt.reg[13]},
	{"rcx",    &_ctxt.reg[14]}, {"rsp",    &_ctxt.reg[15]},
	{"rip",    &_ctxt.reg[16]}, {"eflags", &_ctxt.reg[17]},
	{"cs",     &_ctxt.reg[18]}, {"gs",     &_ctxt.reg[19]},
	{"fs",     &_ctxt.reg[20]}, {"pad0",   &_ctxt.reg[21]},
	{"err",    &_ctxt.reg[22]}, {"trapno", &_ctxt.reg[23]},
	{"oldmsk", &_ctxt.reg[24]}, {"cr2",    &_ctxt.reg[25]},
	{NULL}
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

	frame.pc = REG(X64_RIP);
	frame.sp = REG(X64_RSP);
	frame.ra = REG(X64_RIP);

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

	frame.pc = REG(X64_RIP);
	frame.sp = REG(X64_RSP);
	frame.ra = REG(X64_RIP);

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

	hexdump("Exception Stack", (void*)REG_NM(X64_RSP), 0x100);

	(*pFunc)("\n\n");

	_print_dsm_exc(REG(X64_RIP),16, pFunc);
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
