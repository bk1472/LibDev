/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file threadlib.c
 *
 *	Thread Extension library APIs
 *
 *	@author		Baekwon Choi (baekwon.choi@lge.com)
 *	@version    1.0
 *	@date       2018. 3. 23
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
#include	"cmd_shell.h"
#include	"threadlib.h"
#include	"thread_info.h"
#include	"xlibc.h"
#include	"sig_handle.h"
#include	"symbol.h"
#include	"arch.h"
#include	"pc_key.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{
	void*			start_routine;
	void*			arg;
	pthread_attr_t*	p_attr;
	char*			name;
} thread_entry_t;

/*-----------------------------------------------------------------------------
 *	Static Variables & Function prototypes declarations
------------------------------------------------------------------------------*/
static pthread_create_proc_t 			_org_pthread_create;
static int								_gPthreadHeadSz  = 0;
static thread_info_t*					_thrList[2];
static int								_task_id         = 0;
/**
 * thread 별로 가지고 있는 변수 : 각 thread의 stack 영역에 존재함.
 */
static _TLS_ thread_info_t				threadInfo;
/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/
static pid_t _get_tid(void)
{
	#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
	#endif
	#include <unistd.h>
	#include <sys/syscall.h>
	#include <sys/types.h>
  return (pid_t) syscall (SYS_gettid); /* or __NR_gettid */
}

#include <unistd.h>			//	For mmap()
#include <sys/mman.h>		//	For mmap()
#include <limits.h>

static void __dummy_thread(void * param)
{
	return;
}

static void _check_libpthread(void)
{
	size_t			stackSize = PTHREAD_STACK_MIN;
	void*			sp;
	pthread_t		threadId;
	pthread_attr_t	attr;

	sp = mmap((void *) 0, stackSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	pthread_attr_init(&attr);
	pthread_attr_setstack(&attr, sp , stackSize);
	if (_org_pthread_create(&threadId, &attr, (void *)__dummy_thread, (void *)0) == 0)
	{
		_gPthreadHeadSz = (unsigned long)sp + stackSize - 4 - (unsigned long)threadId;
	}

}

static void* _hook_root_entry(void* arg)
{
	typedef void*(*P_FUNC_t)(void*);
	void*					ret;
	thread_entry_t*			hook_entry = arg;
	P_FUNC_t				p_func     = NULL;
	void*					p_arg      = NULL;
	char*					p_name     = NULL;
	pthread_attr_t			attr;
	size_t					st_size;
	char					dmgleBuf[50];

	pthread_attr_init(&attr);
	pthread_attr_getstacksize(&attr, &st_size);

	p_func = hook_entry->start_routine;
	p_arg  = hook_entry->arg;
	p_name = hook_entry->name;


	free(hook_entry);

	if(p_name == NULL)
	{
		char	*symName;

		if ( 0 != findDmgSymByAddr((uint64_t)p_func, &symName, &dmgleBuf[0], 50))
			p_name = symName;
	}

	regist_thread_info(p_name, (void*)p_func, (void*)p_arg, st_size);
	pthread_attr_destroy(&attr);

	ret = p_func(p_arg);

	/* Delete from thread pool*/
	LS_REMQUE(&threadInfo);

	return ret;
}

static int _hook_pthread_create(pthread_t *restrict thread,
		   pthread_attr_t *restrict attr,
		   void *(*start_routine)(void*), void *restrict arg)
{
	thread_entry_t* hook_entry;

	hook_entry = malloc(sizeof(thread_entry_t));

	hook_entry->start_routine = (void*)start_routine;
	hook_entry->arg           = (void*)arg;
	hook_entry->p_attr        = attr;
	hook_entry->name          = NULL;

	return _org_pthread_create(thread,attr, _hook_root_entry, hook_entry) ;
}

static thread_info_t* _find_thread_info(int tskId, const char* name)
{
	thread_info_t*	p_base = NULL;
	thread_info_t*	p_data = NULL;
	int				mod = 1;

	if (name == NULL)
		mod = 0;

	if (mod == 0)
	{
		if(tskId == -1 || threadInfo.tskId)
			return &threadInfo;
	}
	else
	{
		if (strncmp(name, &threadInfo.name[0], 8) == 0)
		{
			return &threadInfo;
		}
	}

	p_base = LS_BASE(thread_info_t, _thrList);
	p_data = p_base->next;

	while(p_data != p_base)
	{
		if(mod == 0)
		{
			if(p_data->tskId == tskId)
				return p_data;
		}
		else
		{
			if (strncmp(name, &p_data->name[0], 8) == 0)
				return p_data;
		}

		p_data = p_data->next;
	}

	return NULL;
}

static void _show_thread (int tskId)
{
	thread_info_t *p_base = NULL;
	thread_info_t *p_data = NULL;

	p_base = LS_BASE(thread_info_t, _thrList);
	p_data = p_base->next;

	while(p_data != p_base)
	{
		if(tskId != -1 && p_data->tskId != tskId)
		{
			p_data = p_data->next;
			continue;
		}
		rprint1n("^y^TI[%-8.8s] tid=%04x(t=%lx,p=%3d,entry=%p) s[%p-%p:%06x]",
						&p_data->name[0],
						p_data->tskId,
						p_data->attr.thId,
						p_data->attr.tId,
						p_data->attr.p_func,
						(void*)((unsigned long)p_data->usl - p_data->stSz),
						p_data->usl,
						p_data->stSz);
		if(tskId != -1 && p_data->tskId == tskId)
		{
			bool	drvMod = true;
			void*	data;

			if(get_drv_fd() < 0) { drvMod = false; data = (void*)(&p_data->attr.thId); }
			else                 { drvMod = true;  data = (void*)(&p_data->attr.tId);  }

			get_thread_backtrace(data, drvMod);
			break;
		}
		p_data = p_data->next;
	}
}

static int _workF06(int* pInKey)
{
	int rc = 0;

	if(cmd_getDebugMode() == false)
		return 0;

	_show_thread(-1);

	if (rc == -1)
		*pInKey = PC_KEY_F06;

	return rc;
}

#include <sys/wait.h>
static void _sigchld_handler (int sigNo, siginfo_t* p_info, void* p_context)
{
	if (sigNo == SIGCHLD)
	{
		pid_t	pid;
		int		status;
		while((pid = waitpid(-1, &status, WNOHANG)) > 0)
			;
	}
}

/*-----------------------------------------------------------------------------
 *	Global Function Definition
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	API Function Definition
------------------------------------------------------------------------------*/
void init_thread_system(void)
{
	static int _init = 0;

	if(_init)
		return;
	_org_pthread_create = xlibc_reg_hook("pthread_create", _hook_pthread_create);
	_check_libpthread();

	thread_map_init();
	(void)xlibc_reg_cmdkey(PC_KEY_F06, (void*)_workF06);

	LS_INIT(_thrList);

	register_signal(SIGCHLD, _sigchld_handler);
	_init = 1;
}

int get_current (void)
{
	return threadInfo.tskId;
}

pid_t get_tid (int tskId)
{
	thread_info_t* p_thInfo = _find_thread_info(tskId, NULL);
	if(p_thInfo == NULL)
		return -1;

	return p_thInfo->attr.tId;
}

pthread_t get_threadid (int tskId)
{
	thread_info_t* p_thInfo = _find_thread_info(tskId, NULL);
	if(p_thInfo == NULL)
		return -1;
	return p_thInfo->attr.thId;
}

char* get_threadName (int tskId)
{
	thread_info_t* p_thInfo = _find_thread_info(tskId, NULL);
	if(p_thInfo == NULL)
		return NULL;

	if(p_thInfo->name[0] == '\0')
		return NULL;

	return &p_thInfo->name[0];
}

int get_tsk_id_by_name(const char* name)
{
	thread_info_t* p_thInfo = _find_thread_info(-1, name);

	if(p_thInfo == NULL)
		return -1;

	return p_thInfo->tskId;
}

void thread_info_print(int tskId)
{
	_show_thread(tskId);
}

int pthread_create_ext(const char* name, pthread_t *restrict thread,
		   pthread_attr_t *restrict attr,
		   void *(*start_routine)(void*), void *restrict arg)
{
	thread_entry_t* hook_entry;

	hook_entry = malloc(sizeof(thread_entry_t));

	hook_entry->start_routine = (void*)start_routine;
	hook_entry->arg           = (void*)arg;
	hook_entry->p_attr        = attr;
	hook_entry->name          = (char*)name;

	return _org_pthread_create(thread,attr, _hook_root_entry, hook_entry) ;
}

void regist_thread_info(const char* p_name, void* p_func, void* p_arg, size_t stSz)
{
	if((++_task_id) == 0xFFFF )
		_task_id = 1;

	threadInfo.tskId        = _task_id;
	threadInfo.attr.thId    = pthread_self();
	threadInfo.attr.tId     = _get_tid();
	threadInfo.name[0]      = '\0';

	if(p_name) strncpy(&threadInfo.name[0], p_name, THREAD_NAME_SZ);
	else       snprintf(&threadInfo.name[0], THREAD_NAME_SZ, "t.%06x", threadInfo.attr.tId);

	threadInfo.name[THREAD_NAME_SZ] = '\0';
	threadInfo.attr.p_func  = p_func;
	threadInfo.attr.arg     = p_arg;
	threadInfo.usl          = (void*)(_gPthreadHeadSz ? (unsigned long)threadInfo.attr.thId + _gPthreadHeadSz + 4 : 0);
	threadInfo.stSz         = stSz;

	/* Insert thread pool*/
	LS_INS_BEFORE(_thrList, &threadInfo);
}
