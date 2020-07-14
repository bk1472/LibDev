#ifndef	__IPC_COMM_H__
#define	__IPC_COMM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include	<sys/mman.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<unistd.h>

#include	<stdlib.h>
#include	<stdio.h>

extern void		init_comm	(void);
extern void		close_shm	(void);

#ifdef __cplusplus
}
#endif

#endif/*__IPC_COMM_H__*/
