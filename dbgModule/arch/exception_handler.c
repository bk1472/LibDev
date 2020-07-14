/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** exception_handler.c
 *
 *	system exception handler
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
#include	"cmd_shell.h"
#include	"thread.h"
#include	"arch.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define SIG_LOCKUP				SIGSEGV
#define LOCKUP_ERRNO			0x0fffffff


#define EXC_NUM_ULIMT_LOOP	(0)
#define EXC_NUM_SEGV_FAULT	(1)
#define EXC_NUM_ILL_INST	(2)
#define EXC_NUM_FPE_FAULT	(3)
#define EXC_NUM_ABORT		(4)
#define EXC_NUM_BUSERR		(5)
#define EXC_NUM_UNKNOWN		(6)
#define EXC_NUM_MAX			(EXC_NUM_UNKNOWN)

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	External Function prototypes declarations
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Static Variables & Function prototypes declarations
------------------------------------------------------------------------------*/
static char *_exception_msg[] =
{
	"It seems like unlimited loop at",		///< EXC_NUM_ULIMT_LOOP
	"Get segment violation fault at",		///< EXC_NUM_SEGV_FAULT
	"Got Illegal Instruction fault at",		///< EXC_NUM_ILL_INST
	"Got Floating point unit fault at",		///< EXC_NUM_FPE_FAULT
	"Got Abort Signal at",					///< EXC_NUM_ABORT
	"Got BUS Error at",						///< EXC_NUM_BUSERR
	"Got unknown fault",					///< EXC_NUM_UNKNOWN
	NULL
};

/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/
static void _print_exception_info(int sigNo, siginfo_t* p_info)
{
	char*	p_name =getSignalName(sigNo);
	char	sigMsg[100];
	int		sigIdx;

	switch(sigNo)
	{
		case SIGSEGV: sigIdx = (p_info->si_errno == LOCKUP_ERRNO)?EXC_NUM_ULIMT_LOOP:EXC_NUM_SEGV_FAULT; break;
		case SIGILL : sigIdx = EXC_NUM_ILL_INST;  break;
		case SIGFPE : sigIdx = EXC_NUM_FPE_FAULT; break;
		case SIGABRT: sigIdx = EXC_NUM_ABORT;     break;
		case SIGBUS : sigIdx = EXC_NUM_BUSERR;    break;
		default     : sigIdx = EXC_NUM_UNKNOWN;
	}

	rprint0n("^r^Exception Ocurred : %s(%d) ", (p_name)? p_name:"un known", sigNo);
	snprintf(&sigMsg[0],100,"%s %p",_exception_msg[sigIdx],p_info->si_addr);
	rprint1n("^y^%s", sigMsg);

}

static void _exception_handler (int sigNo, siginfo_t* p_info, void* p_context)
{
	_print_exception_info(sigNo, p_info);
	make_registermap(p_context);
	print_context_info(rprint0n);

	cmd_setExcMode(true);
	while (1)
	{
		cmd_setDebugLevel(0);
		cmdDebugMain();
	}
}

/*-----------------------------------------------------------------------------
 *	Global Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
void init_except (void)
{
	register_signal(SIGSEGV,_exception_handler);
	register_signal(SIGILL, _exception_handler);
	register_signal(SIGFPE, _exception_handler);
	register_signal(SIGABRT,_exception_handler);
	register_signal(SIGBUS, _exception_handler);
}
