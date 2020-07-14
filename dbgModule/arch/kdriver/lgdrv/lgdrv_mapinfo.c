/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"include/lgdrv.h"
#include	<asm/atomic.h>
#include	<linux/sched.h>

/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/

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
static int _mmap_info_update(pid_t pid, unsigned long *p_addr, region_t* p_inreg, int lock_flag, int prnt_flag)
{
	struct task_struct*		p_task = NULL;
	struct mm_struct*		p_mm   = NULL;
	struct vm_area_struct*	p_vma  = NULL;

	volatile region_t*		p_reg  = NULL;
	int						rc     = 0;
	volatile int			mm_count;
	volatile int			idx;
	volatile unsigned long	addr   = *p_addr;


	if(addr == 0)
	{
		p_inreg->reg_beg = 0;
		p_inreg->reg_end = 0;
		p_inreg->reg_opt = 0;
		return -EFAULT;
	}

	rcu_read_lock();
	p_task = pid_task(find_pid_ns(pid, &init_pid_ns), PIDTYPE_PID);
	if(p_task)
		get_task_struct(p_task);
	rcu_read_unlock();

	if (p_task == NULL)
	{
		return -EFAULT;
	}

	if ((p_mm = get_task_mm(p_task)) == NULL)
	{
		rc = -EFAULT;
		goto _OUT_MM_ERR;
	}

	if(lock_flag)
		down_read(&p_mm->mmap_sem);
	p_vma = p_mm->mmap;

	if(p_vma == NULL)
	{
		rc = -EFAULT;
		goto _OUT_VMA_ERR;
	}

	mm_count = p_mm->map_count;
	if(mm_count > 0)
	{
		size_t size = sizeof(region_t)*mm_count;

		size = ((size/0x1000)+1)*0x1000;
		p_reg = (region_t*)kmalloc(size, GFP_KERNEL);
		//p_reg = (region_t*)kmalloc(sizeof(region_t)*mm_count, GFP_KERNEL);
	}

	for(idx = 0; idx < mm_count; idx++)
	{
		int	flag;

		flag = p_vma->vm_flags;
		p_reg[idx].reg_beg = p_vma->vm_start;
		p_reg[idx].reg_end = p_vma->vm_end;
		p_reg[idx].reg_opt = flag;
		#if 0
		printk(KERN_INFO "[%03d] %016lx-%016lx %c%c%c%c", idx,
										p_vma->vm_start,
										p_vma->vm_end,
										flag & VM_READ  ? 'r':'-',
										flag & VM_WRITE ? 'w':'-',
										flag & VM_EXEC  ? 'x':'-',
										flag & VM_MAYSHARE  ? 's':'p');
		printk(KERN_INFO "\n");
		#endif
		p_vma = p_vma->vm_next;
	}

	if( addr != 0 && p_inreg != NULL)
	{
		int		c, l = 0, r = mm_count -1, matched = 0;

		do
		{
			c = (l + r) / 2;

			if      ( addr  < p_reg[c].reg_beg)  { matched = 0; r = c - 1; }
			else if ( addr >= p_reg[c].reg_end)  { matched = 0; l = c + 1; }
			else if ( addr >= p_reg[c].reg_beg
				   && addr  < p_reg[c].reg_end)  { matched = 1; l = c + 1; }
			else                                 { matched = 0; l = c + 1; }

		} while((l <= r) && (matched == 0));

		if(matched)
		{
			unsigned long tmp_opt = 0;

			p_inreg->reg_opt = 0;

			tmp_opt = (p_reg[c].reg_opt & VM_READ)? REG_OPT_READ:0;
			p_inreg->reg_opt |= tmp_opt;

			tmp_opt = (p_reg[c].reg_opt & VM_WRITE)? REG_OPT_WRITE:0;
			p_inreg->reg_opt |= tmp_opt;

			tmp_opt = (p_reg[c].reg_opt & VM_EXEC)? REG_OPT_EXEC:0;
			p_inreg->reg_opt |= tmp_opt;

			tmp_opt = (p_reg[c].reg_opt & VM_MAYSHARE)? REG_OPT_SHARED:0;
			p_inreg->reg_opt |= tmp_opt;

			p_inreg->reg_beg = p_reg[c].reg_beg;
			p_inreg->reg_end = p_reg[c].reg_end;
			#if 0
			printk(KERN_INFO "addr:%lx [%lx-%lx]:%x", addr, p_inreg->reg_beg, p_inreg->reg_end, p_inreg->reg_opt);
			#endif
		}
		else
		{
			p_inreg->reg_beg = 0;
			p_inreg->reg_end = 0;
			p_inreg->reg_opt = 0;
			rc = -EFAULT;
		}
	}

	if (prnt_flag)
	{
		for(idx = 0; idx < mm_count; idx++)	{
			printk(KERN_INFO "[%03d] %016lx-%016lx %c%c%c%c", idx,
											p_reg[idx].reg_beg,
											p_reg[idx].reg_end,
											p_reg[idx].reg_opt & REG_OPT_READ    ? 'r':'-',
											p_reg[idx].reg_opt & REG_OPT_WRITE   ? 'w':'-',
											p_reg[idx].reg_opt & REG_OPT_EXEC    ? 'x':'-',
											p_reg[idx].reg_opt & REG_OPT_SHARED  ? 's':'p');
		}
	}
	if(mm_count > 0 && p_reg != NULL)
		kfree((const void*)p_reg);

_OUT_VMA_ERR:
	if(lock_flag)
		up_read(&p_mm->mmap_sem);

_OUT_MM_ERR:
	put_task_struct(p_task);

	return rc;
}

/*-----------------------------------------------------------------------------
 *	API Function prototypes defines
------------------------------------------------------------------------------*/
pid_t get_curr_pid(void)
{
	return task_pid_nr(current);
}

int mmap_update(pid_t pid)
{
	return _mmap_info_update(pid, 0, NULL, 1, 1);
}

int get_region(pid_t pid, unsigned long *addr, region_t* p_reg)
{
	int rc;

	if(p_reg == NULL)
		return -EFAULT;

	rc = _mmap_info_update(pid, addr, p_reg, 1, 0);
	//printk(KERN_INFO "pid(%d) addr:%p, rc:%d", pid, addr, rc);
	if(rc < 0)
	{
		p_reg->reg_beg = 0;
		p_reg->reg_end = 0;
		p_reg->reg_opt = 0;
	}

	return rc;
}
