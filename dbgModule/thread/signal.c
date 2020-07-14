/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** signal.c
 *
 *	signal handler libraries
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
#include	"sig_handle.h"
#include	"threadlib.h"
#include	"backtrace.h"
#include	"xlibc.h"
#include	"arch.h"
#include	"ipc_comm.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define EXCEPTION_STACK_SIZE		(32*1024)
#define USR1_STACK_SIZE				EXCEPTION_STACK_SIZE

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{
	char*			name;
	int				sigNo;
	sig_type_t		type;
	SIG_HADLER_T	p_handler;
	SIG_INSTAL_T	p_install;
} sig_tbl_t;

/*-----------------------------------------------------------------------------
 *	Static Variables & Function prototypes declarations
------------------------------------------------------------------------------*/
static void _null_handler	(int sigNo, siginfo_t* p_siginfo, void* p_frame){}

static void _sig_exception_installer	(int sigNo);
static void _sig_killer_installer		(int sigNo);
static void _sig_cmd_installer			(int sigNo);

#if (USE_SIGSTACK_LVL >= 1)
static char	_exception_stack	[EXCEPTION_STACK_SIZE];
#endif
#if (USE_SIGSTACK_LVL == 2)
static char	_usr1_stack			[USR1_STACK_SIZE];
#endif

static sig_tbl_t _sigTbl[] =
{
	/* exception */
	{ "SIGSEGV", SIGSEGV, SIG_EXCEPT, _null_handler, _sig_exception_installer},
	{ "SIGILL",  SIGILL,  SIG_EXCEPT, _null_handler, _sig_exception_installer},
	{ "SIGFPE",  SIGFPE,  SIG_EXCEPT, _null_handler, _sig_exception_installer},
	{ "SIGABRT", SIGABRT, SIG_EXCEPT, _null_handler, _sig_exception_installer},
	{ "SIGBUS",  SIGBUS,  SIG_EXCEPT, _null_handler, _sig_exception_installer},

	/* terminate */
	{ "SIGINT",  SIGINT,  SIG_KILL,   _null_handler, _sig_killer_installer   },
	{ "SIGCONT", SIGCONT, SIG_KILL,   _null_handler, _sig_killer_installer   },
	{ "SIGSTOP", SIGSTOP, SIG_KILL,   _null_handler, _sig_killer_installer   },
	{ "SIGCHLD", SIGCHLD, SIG_KILL,   _null_handler, _sig_killer_installer   },
	/* command */
	{ "SIGUSR1", SIGUSR1, SIG_CMD,    _null_handler, _sig_cmd_installer      },
	{ "SIGUSR2", SIGUSR2, SIG_CMD,    _null_handler, _sig_cmd_installer      },
	{ NULL }
};
/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/
static void _signal_handler(int sigNo, siginfo_t* p_siginfo, void* p_frame)
{
	int idx = 0;

	while(_sigTbl[idx].name != NULL)
	{
		sig_tbl_t*	p_sig = &_sigTbl[idx];

		if(sigNo == p_sig->sigNo)
		{

			if(p_sig->p_handler != _null_handler)
			{
				p_sig->p_handler(sigNo, p_siginfo, p_frame);
				p_sig->p_install(sigNo);
			}
			break;
		}
		idx++;
	}
}

static void _sig_exception_installer(int sigNo)
{
	struct sigaction	act;

	#if (USE_SIGSTACK_LVL >= 1)
	if(sigNo == SIGSEGV)
	{
		stack_t	newSS, oldSS;

		newSS.ss_sp    = &_exception_stack[0];
		newSS.ss_size  = EXCEPTION_STACK_SIZE;
		newSS.ss_flags = 0;

		sigaltstack(&newSS, &oldSS);
		sigemptyset(&act.sa_mask);
		sigaddset(&act.sa_mask, SIGSEGV);
	}
	#endif
	act.sa_sigaction = _signal_handler;
	act.sa_flags	 = SA_NOCLDSTOP | SA_ONESHOT | SA_SIGINFO | SA_NOMASK | SA_ONSTACK;

	sigaction(sigNo, &act, NULL);
}

static void _sig_killer_installer(int sigNo)
{
	struct sigaction	act;

	act.sa_sigaction = _signal_handler;
	act.sa_flags	 = SA_NOCLDSTOP | SA_ONESHOT | SA_SIGINFO | SA_NOMASK | SA_ONSTACK;

	#if 0
	rprint0n("^G^ This is sig handler: ");
	switch(sigNo)
	{
		case SIGINT : rprint1n("^r^SIGINT");break;
		case SIGCONT: rprint1n("^r^SIGCONT");break;
		case SIGSTOP: rprint1n("^r^SIGSTOP");break;
		case SIGCHLD: rprint1n("^r^SIGCHLD");break;
	}
	#endif
	if(sigNo == SIGINT)
		act.sa_flags |= SA_RESETHAND;

	sigaction(sigNo, &act, NULL);
}

static void _sig_cmd_installer(int sigNo)
{
	struct sigaction	act;

	/**
	 * AARCH64 아키텍쳐에서는
	 * usr1 signal에서 별도의 stack을 사용하면
	 * 그 다음부터 stack 정보가 꼬이는 문제가 발생한다.
	 * architecture 구성의 차이로 보여짐.
	 * 좀더 확인 필요함
	 */
	#if (USE_SIGSTACK_LVL == 2)
	if(sigNo == SIGUSR1)
	{
		stack_t	newSS, oldSS;

		newSS.ss_sp    = &_usr1_stack[0];
		newSS.ss_size  = USR1_STACK_SIZE;
		newSS.ss_flags = 0;

		sigaltstack(&newSS, &oldSS);
		sigemptyset(&act.sa_mask);
		sigaddset(&act.sa_mask, sigNo);
	}
	#endif
	act.sa_sigaction = _signal_handler;
	act.sa_flags	 = SA_NOCLDSTOP | SA_ONESHOT | SA_SIGINFO | SA_NOMASK | SA_ONSTACK;

	sigaction(sigNo, &act, NULL);
}

static void _usr1_cmd_handler (int sigNo, siginfo_t* p_info, void* p_context)
{
	frame_reg_t frame;

	make_registermap(p_context);
	frame = get_frame_sig();

	stack_trace(0x800, 32, NULL, &frame, rprint0n);
}

/*-----------------------------------------------------------------------------
 *	Global Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
int register_signal(int sigNo, SIG_HADLER_T p_handler)
{
	int idx =  0;
	int	rc  = -1;

	while(_sigTbl[idx].name != NULL)
	{
		sig_tbl_t*	p_sig = &_sigTbl[idx];

		if(sigNo == p_sig->sigNo)
		{

			p_sig->p_handler = p_handler;
			if(p_sig->p_handler != _null_handler)
				p_sig->p_install(sigNo);
			rc = 0;
			break;
		}
		idx++;
	}

	return rc;
}

int unregister_signal(int sigNo)
{
	return register_signal(sigNo, _null_handler);
}

char* getSignalName(int sigNo)
{
	int idx =  0;

	while(_sigTbl[idx].name != NULL)
	{
		sig_tbl_t*	p_sig = &_sigTbl[idx];

		if(sigNo == p_sig->sigNo)
			return p_sig->name;
		idx++;
	}

	return NULL;
}

void get_signal_backtrace(void* data)
{
	pthread_t thId = *((pthread_t*)data);

	register_signal(SIGUSR1, _usr1_cmd_handler);
	pthread_kill(thId, SIGUSR1);
	usleep(50000);
}
