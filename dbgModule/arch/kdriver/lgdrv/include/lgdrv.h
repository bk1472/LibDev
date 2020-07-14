#ifndef __LGDRV_H__
#define __LGDRV_H__
/*-----------------------------------------------------------------------------
 *	Control Constants
------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/vermagic.h>
#include <linux/fcntl.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/pid.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <asm/thread_info.h>
#include <linux/slab.h>

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/
#define REG_OPT_READ			0x01
#define	REG_OPT_WRITE			0x02
#define	REG_OPT_EXEC			0x04
#define REG_OPT_SHARED			0x08

/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef struct
{
	unsigned long	reg_beg;
	unsigned long	reg_end;
	unsigned long	reg_opt;
} region_t;

typedef struct
{
	pid_t			pid;	/* thread tid or process pid          */
	unsigned long	pc;		/* program counter (execution pointer */
	unsigned long	ra;		/* return address                     */
	unsigned long	sp;		/* current stack pointer              */
} context_t;

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/
#define LG_DRV_IOCTL_NUM		('L')
#define LG_READ_CONTEXT			_IOWR(LG_DRV_IOCTL_NUM, 0, context_t)

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 * External Used functions
------------------------------------------------------------------------------*/
/* lgdrv_mapinfo.c */
extern	pid_t		get_curr_pid		(void);
extern	int			mmap_update			(pid_t pid);
extern	int			get_region			(pid_t pid, unsigned long* addr, region_t* reg);

/* lgdrv_sp.c */
extern	long		build_context_frame	(context_t* p_context, struct task_struct** pp);

#ifdef __cplusplus
}
#endif

#endif/*__LGDRV_H__*/
