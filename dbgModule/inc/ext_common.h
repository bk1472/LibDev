/******************************************************************************
 *	 ROBOT LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 *	 Copyright(c) 2018 by LG Electronics Inc.
 *
 *	 All rights reserved. No part of this work may be reproduced, stored in a
 *	 retrieval system, or transmitted by any means without prior written
 *	 permission of LG Electronics Inc.
 *****************************************************************************/

/** @file ext_common.h
 *
 *	Extension Libraries Common header file
 *
 *	@author 	Baekwon Choi (baekwon.choi@lge.com)
 *	@version	1.0
 *	@date		2018. 3. 19
 *	@note
 *	@see
 */
#ifndef __EXT_COMMON_H__
#define __EXT_COMMON_H__
/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	<unistd.h>


#include	<stdio.h>
#include	<stdlib.h>
#include	<stdarg.h>
#include	<string.h>
#include	<stdint.h>

#include	<pthread.h>

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define	EXT_CMN_BUF_SZ			(256)

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/
#define EXPORT					extern __attribute__((visibility("default")))
#define __INIT__				__attribute__((constructor))
#define __EXIT__				__attribute__((destructor))
#define __CONT__(_n)			__attribute__((constructor(_n)))
#define __DEST__(_n)			__attribute__((destructor(_n)))
#define _TLS_					__thread
#define TLS_VALID(tls)			((unsigned long *)(&tls) > (&_end) )
#define STRONG_ALIAS(_n, _an)	extern __typeof (_n) _an __attribute__ ((alias (#_n)));
#define WEAK_ALIAS(_n, _an)		extern __typeof (_n) _an __attribute__ ((weak, alias (#_n)));
#define _WEAK_					__attribute__((weak))
#define _USER_


#define __START					(uint64_t)_start
#define __END					(uint64_t)_end

#ifdef __cplusplus
#define	VARG_PROTO	...
#else
#define	VARG_PROTO
#endif	/* __cplusplus */

#ifdef __cplusplus
#define restrict
#endif

#define	LS_ISEMPTY(listp)													\
		(((lst_t *)(listp))->ls_next == (lst_t *)(listp))

#define	LS_INIT(listp) {													\
		((lst_t *)(&(listp)[0]))->ls_next = 								\
		((lst_t *)(&(listp)[0]))->ls_prev = ((lst_t *)(&(listp)[0]));		\
}

#define	LS_INS_BEFORE(oldp, newp) {											\
		((lst_t *)(newp))->ls_prev = ((lst_t *)(oldp))->ls_prev;			\
		((lst_t *)(newp))->ls_next = ((lst_t *)(oldp));						\
		((lst_t *)(newp))->ls_prev->ls_next = ((lst_t *)(newp));			\
		((lst_t *)(newp))->ls_next->ls_prev = ((lst_t *)(newp));			\
}

#define	LS_INS_AFTER(oldp, newp) {											\
		((lst_t *)(newp))->ls_next = ((lst_t *)(oldp))->ls_next;			\
		((lst_t *)(newp))->ls_prev = ((lst_t *)(oldp));						\
		((lst_t *)(newp))->ls_next->ls_prev = ((lst_t *)(newp));			\
		((lst_t *)(newp))->ls_prev->ls_next = ((lst_t *)(newp));			\
}

#define	LS_REMQUE(remp) {													\
	if (!LS_ISEMPTY(remp)) {												\
		((lst_t *)(remp))->ls_prev->ls_next = ((lst_t *)(remp))->ls_next;	\
		((lst_t *)(remp))->ls_next->ls_prev = ((lst_t *)(remp))->ls_prev;	\
		LS_INIT(remp);														\
	}																		\
}

#define LS_BASE(type, basep) ((type *) &(basep)[0])


#define LS_HEAD_DECLARE		lst_t * ls_prev ; lst_t* ls_next ; char * container_type = "list";

#if 0
/*---	Example of doubly linked list	---*/

		typedef struct LINKED_LIST_tag {
			struct LINKED_LIST_tag	*next;
			struct LINKED_LIST_tag	*prev;

			int	data;
		} LINKED_LIST;

		LINKED_LIST	*list[2];		//	declaration of header
		LS_INIT(list);			//	initialize

		LINKED_LIST *list_base = LS_BASE(LINKED_LIST, list);
		//LINKED_LIST *list_base = (LINKED_LIST *) &list[0]; //	type cast - (*)

		LINKED_LIST *list_data = list_base->next;

		while (list_data != list_base) {
			//	traverse
			list_data = list_data->next;
		}

	(*)		*list[0] --> *next, *list[1] --> *prev
			i.e. Header has only links without data.
#endif	/* __COMMENT_OUT */


#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
//----------------------------------------------------------------------------
//	Macro for initializing doubly linked list header
//----------------------------------------------------------------------------
typedef	struct ls_elt {
	struct ls_elt	*ls_next;
	struct ls_elt	*ls_prev;
} lst_t;

//----------------------------------------------------------------------------
//	Common hooking function type
//----------------------------------------------------------------------------
typedef int		(*system_proc_t)(const char * cmd);
typedef void *	(*malloc_proc_t)(size_t size) ;
typedef void 	(*free_proc_t)	(void *ptr);
typedef void* 	(*calloc_proc_t)(size_t nmemb, size_t size);
typedef void*	(*realloc_proc_t)(void *ptr, size_t size);
typedef void *  (*memalign_proc_t)(size_t alignment, size_t bytes);
typedef int 	(*posix_memalign_proc_t)(void **memptr, size_t alignment, size_t size);
typedef int		(*printf_proc_t)(const char* fmt, ...);
typedef int		(*fprintf_proc_t)(FILE* stream, const char* fmt, ...);
typedef int 	(*vprintf_proc_t)(const char *format, va_list ap);
typedef int 	(*puts_proc_t)(const char *s);
typedef int 	(*vfprintf_proc_t)(FILE *stream, const char *format, va_list ap);
typedef int 	(*fputs_proc_t)(const char *s, FILE *stream);

typedef int		(*pthread_create_proc_t)(	pthread_t *restrict thread,
		   									const pthread_attr_t *restrict attr,
		   									void *(*start_routine)(void*),
											void *restrict arg);



/*-----------------------------------------------------------------------------
 * External Used variables
------------------------------------------------------------------------------*/
extern unsigned long	_start[];
extern unsigned long	_end;

extern int				get_drv_fd		(void);
extern void				null_function	();

#ifdef __cplusplus
}
#endif

#ifndef __cplusplus

# ifndef __bool
# define __bool
typedef unsigned int	bool;
# endif

# ifndef true
# define true	1
# endif

# ifndef false
# define false	0
# endif

#endif

#endif/*__EXT_COMMON_H__*/
