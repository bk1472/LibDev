#include	"include/lgdrv.h"

long build_context_frame(context_t* p_context, struct task_struct** pp)
{
	volatile struct pt_regs*		r;
	volatile struct task_struct*	p = *pp;

	r = (struct pt_regs*)task_pt_regs(p);

	#ifdef __AARCH64__
	#if 0
	printk(KERN_INFO "regs[ 0] = %016lx\n",(unsigned long)r->regs[ 0]);
	printk(KERN_INFO "regs[ 1] = %016lx\n",(unsigned long)r->regs[ 1]);
	printk(KERN_INFO "regs[ 2] = %016lx\n",(unsigned long)r->regs[ 2]);
	printk(KERN_INFO "regs[ 3] = %016lx\n",(unsigned long)r->regs[ 3]);
	printk(KERN_INFO "regs[ 4] = %016lx\n",(unsigned long)r->regs[ 4]);
	printk(KERN_INFO "regs[ 5] = %016lx\n",(unsigned long)r->regs[ 5]);
	printk(KERN_INFO "regs[ 6] = %016lx\n",(unsigned long)r->regs[ 6]);
	printk(KERN_INFO "regs[ 7] = %016lx\n",(unsigned long)r->regs[ 7]);
	printk(KERN_INFO "regs[ 8] = %016lx\n",(unsigned long)r->regs[ 8]);
	printk(KERN_INFO "regs[ 9] = %016lx\n",(unsigned long)r->regs[ 9]);
	printk(KERN_INFO "regs[10] = %016lx\n",(unsigned long)r->regs[10]);
	printk(KERN_INFO "regs[11] = %016lx\n",(unsigned long)r->regs[11]);
	printk(KERN_INFO "regs[12] = %016lx\n",(unsigned long)r->regs[12]);
	printk(KERN_INFO "regs[13] = %016lx\n",(unsigned long)r->regs[13]);
	printk(KERN_INFO "regs[14] = %016lx\n",(unsigned long)r->regs[14]);
	printk(KERN_INFO "regs[15] = %016lx\n",(unsigned long)r->regs[15]);
	printk(KERN_INFO "regs[16] = %016lx\n",(unsigned long)r->regs[16]);
	printk(KERN_INFO "regs[17] = %016lx\n",(unsigned long)r->regs[17]);
	printk(KERN_INFO "regs[18] = %016lx\n",(unsigned long)r->regs[18]);
	printk(KERN_INFO "regs[19] = %016lx\n",(unsigned long)r->regs[19]);
	printk(KERN_INFO "regs[20] = %016lx\n",(unsigned long)r->regs[20]);
	printk(KERN_INFO "regs[21] = %016lx\n",(unsigned long)r->regs[21]);
	printk(KERN_INFO "regs[22] = %016lx\n",(unsigned long)r->regs[22]);
	printk(KERN_INFO "regs[23] = %016lx\n",(unsigned long)r->regs[23]);
	printk(KERN_INFO "regs[24] = %016lx\n",(unsigned long)r->regs[24]);
	printk(KERN_INFO "regs[25] = %016lx\n",(unsigned long)r->regs[25]);
	printk(KERN_INFO "regs[26] = %016lx\n",(unsigned long)r->regs[26]);
	printk(KERN_INFO "regs[27] = %016lx\n",(unsigned long)r->regs[27]);
	printk(KERN_INFO "regs[28] = %016lx\n",(unsigned long)r->regs[28]);
	printk(KERN_INFO "regs[29] = %016lx\n",(unsigned long)r->regs[29]);
	printk(KERN_INFO "regs[30] = %016lx\n",(unsigned long)r->regs[30]);
	printk(KERN_INFO "sp       = %016lx\n",(unsigned long)r->sp      );
	printk(KERN_INFO "pc       = %016lx\n",(unsigned long)r->pc      );
	printk(KERN_INFO "pstate   = %016lx\n",(unsigned long)r->pstate  );
	#endif

	p_context->pc = r->pc;
	p_context->sp = r->sp;
	p_context->ra = r->regs[14];
	#endif
	#ifdef __X86_64__
	#if 0
	printk(KERN_INFO "r15     = %016lx\n",r->r15    );
	printk(KERN_INFO "r14     = %016lx\n",r->r14    );
	printk(KERN_INFO "r13     = %016lx\n",r->r13    );
	printk(KERN_INFO "r12     = %016lx\n",r->r12    );
	printk(KERN_INFO "bp      = %016lx\n",r->bp     );
	printk(KERN_INFO "bx      = %016lx\n",r->bx     );
	printk(KERN_INFO "r11     = %016lx\n",r->r11    );
	printk(KERN_INFO "r10     = %016lx\n",r->r10    );
	printk(KERN_INFO "r9      = %016lx\n",r->r9     );
	printk(KERN_INFO "r8      = %016lx\n",r->r8     );
	printk(KERN_INFO "ax      = %016lx\n",r->ax     );
	printk(KERN_INFO "cx      = %016lx\n",r->cx     );
	printk(KERN_INFO "dx      = %016lx\n",r->dx     );
	printk(KERN_INFO "si      = %016lx\n",r->si     );
	printk(KERN_INFO "di      = %016lx\n",r->di     );
	printk(KERN_INFO "orig_ax = %016lx\n",r->orig_ax);
	printk(KERN_INFO "ip      = %016lx\n",r->ip     );
	printk(KERN_INFO "cs      = %016lx\n",r->cs     );
	printk(KERN_INFO "flags   = %016lx\n",r->flags  );
	printk(KERN_INFO "sp      = %016lx\n",r->sp     );
	printk(KERN_INFO "ss      = %016lx\n",r->ss     );
	#endif

	p_context->pc = r->ip;
	p_context->sp = r->sp;
	p_context->ra = r->bx; // ???
	#endif

	printk(KERN_INFO "pid:%d, pc(%016lx), sp(%016lx), fp(%016lx)\n",
			p_context->pid,
			p_context->pc,
			p_context->sp,
			p_context->ra);

	return sizeof(context_t);
}
