/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** backtrace.c
 *
 *	stack backtrace routine
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 4. 4
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
#include	"backtrace.h"
#include	"arch.h"

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
static char	trace_fmt1[] = ">>    [%03d: sp=%p, pc=%p]\n";
static char	trace_fmt2[] = ">>    [%03d: sp=%p, pc=%p(%p+%#04x), %s]\n";
static char	trace_fmt3[] = ">>    [%03d: sp=%p, pc=%p,%s()@%s:%d]\n";
static char	trace_fmt4[] = ">>    [%03d: sp=%p, pc=%p,%s@%s:%d]\n";
static char	trace_fmt5[] = ">>    [%03d: sp=%p, pc=%p(%p+%#04x), %s()@%s]\n";

/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/
static void _printTracedFrame(int idx, uint64_t* tSp, uint64_t* tPc, printf_proc_t pFunc, int dSz)
{
	if (is_symbol_load() == true)
	{
		char		*pFileName;
		char		*symName;
		uint64_t	symValue;
		int			lineNo;
		char		dmgleBuf[dSz];	/* over C99 */

		if (   (false == isValidAddr((uint64_t)tSp))
			|| (false == isValidPc((uint64_t)tPc))
		   )
			return;

		if( (symValue = findDmgSymByAddr((uint64_t)tPc, &symName, &dmgleBuf[0], dSz)) != 0)
		{
			bool cpp_flag = false;

			if(symName == &dmgleBuf[0])
				cpp_flag = true;

			if(NULL != getLibName())
			{
				(*pFunc)(trace_fmt5, idx, tSp, tPc, symValue, (uint64_t)tPc-symValue, symName, getLibName());
				return;
			}

			if( (lineNo = addr2line((uint64_t)tPc, &pFileName)) == 0)
			{
				(*pFunc)(trace_fmt2, idx, tSp, tPc, symValue, (uint64_t)tPc-symValue, symName);
			}
			else
			{
				char* _tr_fmt;
				/**
				 * BK1472 comment:
				 *  C++이 demangle 되면 return 되는 pointer 와 입렫 pointer 가 같다.
				 */
				if(cpp_flag == true) _tr_fmt = trace_fmt4;
				else                 _tr_fmt = trace_fmt3;
				(*pFunc)(_tr_fmt, idx, tSp, tPc, symName, pFileName, lineNo);
			}
			return;
		}
	}
	(*pFunc)(trace_fmt1, idx, tSp, tPc);
}

static void get_driver_backtrace(void* data)
{
	pid_t		pid = *((pid_t*)data);
	frame_reg_t	frame;

	frame = get_frame_drv(pid);

	stack_trace(0x2000, 32, NULL, &frame, rprint0n);
}

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
void log_stack(uint64_t* log, void* p_regs, int size)
{
	uint64_t	*sp, *ra, *pc, *susl;
	frame_reg_t *frame;
	int32_t		fs;
	int			n;

	if(p_regs)
	{
		frame = (frame_reg_t*)p_regs;
		pc = (uint64_t*)frame->pc;
		sp = (uint64_t*)frame->sp;
		ra = (uint64_t*)frame->ra;
	}
	else
	{
		str_pc(&pc);
		str_sp(&sp);
		str_ra(&ra);
	}

	susl = sp + 0x2000;
	for(n = 0; n < size; n++)
	{
		if((fs = get_frame_size(sp, susl, pc, &ra)) == 0)
			break;
		log[n] = (uint64_t)pc;
		sp = sp + (fs/sizeof(uint64_t));
		pc = ra;
	}
}

void stack_trace(uint32_t size, uint32_t count, uint64_t* log, void* p_regs, printf_proc_t pFunc)
{
	uint64_t		*sp, *limit;
	uint64_t*		ra;
	uint64_t*		pc   = NULL;
	uint64_t*		susl = NULL;
	region_t		mReg = {0,};
	int32_t			fs   = 0;
	int				n    = 0;

	/**
	 * Save First Call return addres
	 */
	str_ra(&ra);

	if (pFunc == NULL)
		pFunc = rprint0n;

	if(p_regs == NULL)
	{
		str_sp(&sp);
		str_pc(&pc);
	}
	else
	{
		frame_reg_t *frame = (frame_reg_t*)p_regs;
		pc = (uint64_t*)frame->pc;
		sp = (uint64_t*)frame->sp;
		ra = (uint64_t*)frame->ra;
	}
	mReg = getRegionEntry((uint64_t)sp);

	if(mReg.reg_end !=0)
		susl = (uint64_t*)mReg.reg_end;

	if(isValidPc((uint64_t)pc) == false)
	{
		/**
		 * 만약 유효하지 않은 PC면  RA가 PC를 대신 한다.
		 */
		if(NULL == log)
			(*pFunc)("\tChanging pc: %p -> %p\n", pc, ra);
		pc = ra;
	}
	else if (isValidPc((uint64_t)ra) == true)
	{
		/**
		 * TODO:
		 *	유효한 PC이지만 3개이상 추적이 불가능할 경우 stack이 깨진 것으로 가정하고
		 *	RA를 대신 사용 한다.
		 *	Stack 이 Crash 되었을 때 의미가 있으므로 나중에 추가 작업 한다.
		 *
		 */
	}

	if(NULL == log)
	{
		if( mReg.reg_beg != 0)
			(*pFunc) (">>\tbacktrace for functions, stack[0x%lx-0x%lx]\n", mReg.reg_beg, mReg.reg_end);
		else
			(*pFunc) (">>\tbacktrace for functions\n");
	}

	if(NULL == log)
	{
		_printTracedFrame(n, sp, pc, pFunc, 50/*demangle size*/);
		n = 1;
	}

	if(size > 0)
	{
		limit = sp + (size / sizeof(uint64_t));

	}
	else
	{
		limit = susl;
	}
	if(limit >= susl)
		limit = susl;

	do
	{
		if((fs = get_frame_size(sp, susl, pc, &ra)) == 0)
			break;

		if (NULL != log)
		{
			if (n == STACK_LOG_DEPTH)
				break;
			log[n] = (uint64_t)pc;
		}
		else
		{
			_printTracedFrame(n, sp, ra, pFunc, 50/*demangle size*/);
		}
		sp = sp + (fs/sizeof(uint64_t));
		pc = ra;
		n++;
	} while(sp < limit);

	return;
}

void print_stack(void)
{
	uint64_t	*pc, *sp, *ra;
	frame_reg_t reg;

	str_pc(&pc);
	str_ra(&ra);
	str_sp(&sp);

	reg.pc = (uint64_t) pc;     /* program counter (execution pointer */
	reg.ra = (uint64_t) ra;     /* return address                     */
	reg.sp = (uint64_t) sp;     /* current stack pointer              */
	stack_trace(0x2000, 32, NULL, &reg, NULL);
	#if 0 // log call test
	{
		int			i;
		uint64_t	log[10];

		log_stack(&log[0], &reg, STACK_LOG_DEPTH);


		for(i = 0; i < STACK_LOG_DEPTH;i ++)
		{
			rprint1n("^p^[%d]: %lx", i, log[i]);
		}
	}
	#endif
}

void get_thread_backtrace(void* data, bool drvMod)
{
	bool	self = false;

	if (drvMod == true)
	{
		if((get_tid(get_current())) ==  *((pid_t*)data))
			self = true;
	}
	else
	{
		if((get_threadid(get_current())) == *((pthread_t*)data))
			self = true;
	}

	if(self == true)
	{
		stack_trace(0x2000, 32, NULL, NULL, NULL);
		return;
	}

	if(drvMod == false) get_signal_backtrace(data);
	else                get_driver_backtrace(data);
}
