/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file xlibc_hook_func.c
 *
 *  Hooking Function compilations
 *
 *	@author 	Baekwon Choi (baekwon.choi@lge.com)
 *	@version	1.0
 *	@date		2018. 3. 19
 *	@note
 *	@see
 */

/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/
#ifdef __HOOK_LIB__

/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"ext_common.h"
#include	"xlibc.h"

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define HOOKLIST_CNT	(sizeof(_hookingFunc)/sizeof(_hookingFunc[0]))

/*-----------------------------------------------------------------------------
 * Global Variables Declarations
------------------------------------------------------------------------------*/
bool OptTMInfo_Print = false;

/*-----------------------------------------------------------------------------
 * Static Variables Declarations
------------------------------------------------------------------------------*/
static HOOK_LIST_t _hookingFunc[] =
{
	{"system",         NULL},
	{"puts",           NULL},
	{"vprintf",        NULL},
	{"vfprintf",       NULL},
	{"fputs",          NULL},
	{"pthread_create", NULL},
};

static vprintf_proc_t 	_xlibc_orgvprintf;
static puts_proc_t 		_xlibc_orgputs;
static vfprintf_proc_t	_xlibc_orgvfprintf;
static fputs_proc_t		_xlibc_orgfputs;

/*-----------------------------------------------------------------------------
 * Static Function Definition
------------------------------------------------------------------------------*/
static int _xlibc_newvprintf(const char* fmt, va_list ap)
{
	if (xlibc_getPrintStat() != PRINT_LVL_ALL)
		return 0;

	if(OptTMInfo_Print)
		return ext_vprintf(fmt, ap);
	return _xlibc_orgvprintf(fmt, ap);
}

static int _xlibc_newputs(const char* str)
{
	if (xlibc_getPrintStat() != PRINT_LVL_ALL)
		return 0;

	return _xlibc_orgputs(str);
}

static int _xlibc_newvfprintf(FILE* stream, const char* fmt, va_list ap)
{
	if (xlibc_getPrintStat() != PRINT_LVL_ALL)
		return 0;

	return _xlibc_orgvfprintf(stream, fmt, ap);
}

static int _xlibc_newfputs(const char* str, FILE* stream)
{
	if (xlibc_getPrintStat() != PRINT_LVL_ALL)
		return 0;

	return _xlibc_orgfputs(str, stream);
}

/*-----------------------------------------------------------------------------
 * API Function Definition
------------------------------------------------------------------------------*/
int xlibc_getHookList(HOOK_LIST_t** pp_list)
{
	*pp_list = &_hookingFunc[0];

	return HOOKLIST_CNT;
}

void xlibc_backupHook(void)
{
	static int hooked = 0;

	if(hooked)
		return;

	_xlibc_orgvprintf  = xlibc_reg_hook("vprintf"  , _xlibc_newvprintf  );
	_xlibc_orgputs     = xlibc_reg_hook("puts"     , _xlibc_newputs     );
	_xlibc_orgvfprintf = xlibc_reg_hook("vfprintf" , _xlibc_newvfprintf );
	_xlibc_orgfputs    = xlibc_reg_hook("vfputs"   , _xlibc_newfputs    );

	hooked = 1;
}

/*-----------------------------------------------------------------------------
 * Global Hooking LIBC Function Definition
------------------------------------------------------------------------------*/
EXPORT int printf(const char* fmt, ...)
{
	static vprintf_proc_t	p_vprintf = NULL;;
	const char* 			p_name    = "vprintf";
	va_list					ap;
	int						rc;

	if (!xlibc_check_hook())
		return 0;

	if (p_vprintf == NULL)
	{
		if((p_vprintf = xlibc_getHookFunc(p_name)) == NULL)
			p_vprintf = xlibc_getOrigFunc(p_name);
	}
	if(p_vprintf != NULL)
	{
		va_start(ap, fmt);
		rc = p_vprintf(fmt, ap);
		va_end(ap);
	}
	else
		rc = 0;
	return rc;
}

EXPORT int ___printf_chk(int flag, const char* fmt, ...)
{
	#ifndef _IO_FLAGS2_FORTIFY
	#define _IO_FLAGS2_FORTIFY	4
	#endif
	static vprintf_proc_t	p_vprintf = NULL;;
	const char* 			p_name    = "vprintf";
	va_list					ap;
	int						rc;

	if (!xlibc_check_hook())
		return 0;

	flockfile(stdout);
	if (flag > 0)
		stdout->_flags2 |= _IO_FLAGS2_FORTIFY;

	if (p_vprintf == NULL)
	{
		if((p_vprintf = xlibc_getHookFunc(p_name)) == NULL)
			p_vprintf = xlibc_getOrigFunc(p_name);
	}
	if(p_vprintf != NULL)
	{
		va_start(ap, fmt);
		rc = p_vprintf(fmt, ap);
		va_end(ap);
	}
	else
		rc = 0;
	if (flag > 0)
		stdout->_flags2 &= ~_IO_FLAGS2_FORTIFY;

	funlockfile(stdout);

	return rc;
}
STRONG_ALIAS(___printf_chk, __printf_chk)

EXPORT int vprintf(const char* fmt, va_list ap)
{
	static vprintf_proc_t	p_vprintf  = NULL;
	const char* 			p_name     = __func__;
	int						rc;



	if (!xlibc_check_hook())
		return 0;

	if (p_vprintf == NULL)
	{
		if((p_vprintf = xlibc_getHookFunc(p_name)) == NULL)
			p_vprintf = xlibc_getOrigFunc(p_name);
	}

	if(p_vprintf != NULL)
		rc = p_vprintf(fmt, ap);
	else
		rc = 0;

	return rc;
}

EXPORT int ___vprintf_chk(int flag, const char* fmt, va_list ap)
{
	#ifndef _IO_FLAGS2_FORTIFY
	#define _IO_FLAGS2_FORTIFY	4
	#endif
	int		rc;

	if (!xlibc_check_hook())
		return 0;

	flockfile(stdout);
	if (flag > 0)
		stdout->_flags2 |= _IO_FLAGS2_FORTIFY;

	rc = vfprintf(stdout, fmt, ap);

	if (flag > 0)
		stdout->_flags2 &= ~_IO_FLAGS2_FORTIFY;

	funlockfile(stdout);

	return rc;

}
STRONG_ALIAS(___vprintf_chk, __vprintf_chk)

EXPORT int puts(const char* str)
{
	const char* 	p_name = __func__;
	static void*	p_puts = NULL;
	int				rc;

	if (!xlibc_check_hook())
		return 0;

	if (p_puts == NULL)
	{
		if((p_puts = xlibc_getHookFunc(p_name)) == NULL)
			p_puts = xlibc_getOrigFunc(p_name);
	}
	if (p_puts != NULL)
		rc = ((puts_proc_t)p_puts)(str);
	else
		rc = 0;
	return rc;
}

EXPORT int fprintf(FILE* stream, const char* fmt, ...)
{
	static vfprintf_proc_t	p_vfprintf = NULL;;
	const char* 			p_name     = "vfprintf";
	va_list					ap;
	int						rc;

	if (!xlibc_check_hook())
		return 0;

	if (p_vfprintf == NULL)
	{
		if((p_vfprintf = xlibc_getHookFunc(p_name)) == NULL)
			p_vfprintf = xlibc_getOrigFunc(p_name);
	}
	if(p_vfprintf != NULL)
	{
		va_start(ap, fmt);
		rc = p_vfprintf(stream, fmt, ap);
		va_end(ap);
	}
	else
		rc = 0;

	return rc;
}

EXPORT int ___fprintf_chk(FILE* stream, int flag, const char* fmt, ...)
{
	#ifndef _IO_FLAGS2_FORTIFY
	#define _IO_FLAGS2_FORTIFY	4
	#endif
	static vfprintf_proc_t	p_vfprintf = NULL;;
	const char* 			p_name     = "vfprintf";
	va_list					ap;
	int						rc;

	if (!xlibc_check_hook())
		return 0;

	flockfile(stream);
	if (flag > 0)
		stream->_flags2 |= _IO_FLAGS2_FORTIFY;

	if (p_vfprintf == NULL)
	{
		if((p_vfprintf = xlibc_getHookFunc(p_name)) == NULL)
			p_vfprintf = xlibc_getOrigFunc(p_name);
	}
	if(p_vfprintf != NULL)
	{
		va_start(ap, fmt);
		rc = p_vfprintf(stream, fmt, ap);
		va_end(ap);
	}
	else
		rc = 0;
	if (flag > 0)
		stream->_flags2 &= ~_IO_FLAGS2_FORTIFY;

	funlockfile(stream);

	return rc;
}
STRONG_ALIAS(___fprintf_chk, __fprintf_chk)

EXPORT int vfprintf(FILE* stream, const char* fmt, va_list ap)
{
	static vfprintf_proc_t	p_vfprintf = NULL;
	const char* 			p_name     = __func__;
	int						rc;

	if (!xlibc_check_hook())
		return 0;

	if (p_vfprintf == NULL)
	{
		if((p_vfprintf = xlibc_getHookFunc(p_name)) == NULL)
			p_vfprintf = xlibc_getOrigFunc(p_name);
	}

	if(p_vfprintf != NULL)
		rc = p_vfprintf(stream, fmt, ap);
	else
		rc = 0;

	return rc;
}
STRONG_ALIAS(vfprintf, local_vfprintf)

EXPORT int ___vfprintf_chk(FILE *fp, int flag, const char* fmt, va_list ap)
{
	#ifndef _IO_FLAGS2_FORTIFY
	#define _IO_FLAGS2_FORTIFY	4
	#endif
	int		rc;

	if (!xlibc_check_hook())
		return 0;

	flockfile(fp);
	if (flag > 0)
		fp->_flags2 |= _IO_FLAGS2_FORTIFY;

	rc = local_vfprintf(fp, fmt, ap);

	if (flag > 0)
		fp->_flags2 &= ~_IO_FLAGS2_FORTIFY;

	funlockfile(fp);

	return rc;

}
STRONG_ALIAS(___vfprintf_chk, __vfprintf_chk)


EXPORT int fputs(const char* str, FILE* stream)
{
	const char* 	p_name  = __func__;
	static void*	p_fputs = NULL;
	int				rc;

	if (!xlibc_check_hook())
		return 0;

	if (p_fputs == NULL)
	{
		if((p_fputs = xlibc_getHookFunc(p_name)) == NULL)
			p_fputs = xlibc_getOrigFunc(p_name);
	}
	if (p_fputs != NULL)
		rc = ((fputs_proc_t)p_fputs)(str, stream);
	else
		rc = 0;

	return rc;
}

EXPORT int pthread_create(	pthread_t *restrict thread,
							const pthread_attr_t *restrict attr,
							void*(*start_routine)(void*),
							void *restrict arg)
{
	static pthread_create_proc_t	p_pthread_create = NULL;
	const char* 					p_name           = __func__;

	if (!xlibc_check_hook())
		return 0;

	if (p_pthread_create == NULL)
	{
		if((p_pthread_create = xlibc_getHookFunc(p_name)) == NULL)
			p_pthread_create = xlibc_getOrigFunc(p_name);
	}

	return (p_pthread_create(thread, attr, start_routine, arg));
}

#endif/*__HOOK_LIB__*/
