/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/
#include	"include/lgdrv.h"

/*-----------------------------------------------------------------------------
 *	#include Files
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Constant Definitions
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *	Macro Definitions
------------------------------------------------------------------------------*/
#define LGDRV_NAME					"lgdriver"


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
/*-----------------------------------------------------------------------------
 *	Type Definitions
------------------------------------------------------------------------------*/
typedef	struct ls_elt {
	struct ls_elt	*ls_next;
	struct ls_elt	*ls_prev;
} lst_t;

/* for using linked list */
typedef struct _reg_info
{
	struct _reg_info*	next;
	struct _reg_info*	prev;

	pid_t				pid;
	unsigned long		addr;
	region_t			reg;
} reg_info_t;

/*-----------------------------------------------------------------------------
 *	Static Variables declarations
------------------------------------------------------------------------------*/
static dev_t			lgdrv_id;
static struct cdev		lgdrv_cdev;
static struct class*	lgdrv_class;
static struct device*	lgdrv_dev;
static DEFINE_MUTEX(lgioctl_lock);



/* Local Data structure and data */
static reg_info_t*		_regInfoList[2];

/*-----------------------------------------------------------------------------
 *	Static Function prototypes declarations
------------------------------------------------------------------------------*/
static int				lgdrv_open(struct inode *node, struct file *fp);
static loff_t			lgdrv_llseek(struct file *file, loff_t offset, int whence);
static ssize_t			lgdrv_read(struct file *fp, char *buf, size_t cnt, loff_t *pos);
static ssize_t			lgdrv_write(struct file *fp, const char *buf, size_t cnt, loff_t *pos);
static long				lgdrv_ioctl(struct file * file, unsigned int cmd, unsigned long arg);
static int				lgdrv_open(struct inode *node, struct file *fp);
static int				lgdrv_release(struct inode *node, struct file *fp);

/*-----------------------------------------------------------------------------
 *	Static Function Definition
------------------------------------------------------------------------------*/
static int lgdrv_open(struct inode *node, struct file *fp)
{
	if(!try_module_get(THIS_MODULE))
	{
		printk(KERN_INFO "lg driver open call error\n");
		return -ENODEV;
	}
	printk(KERN_INFO "lg driver open called\n");
	return 0;
}

static loff_t lgdrv_llseek(struct file *file, loff_t offset, int whence)
{
	unsigned long	addr = offset;
	pid_t			pid;
	int				rc;
	reg_info_t*		p_data;
	region_t		reg;


	pid = get_curr_pid();

	rc  = get_region(pid, &addr, &reg);

//	printk(KERN_INFO "pid: %d addr:%016lx rc:%d", pid, addr, rc);
	if(rc == 0)
	{
		p_data = (reg_info_t*)kmalloc(sizeof(reg_info_t), GFP_KERNEL);

		p_data->pid         = pid;
		p_data->addr        = addr;
		p_data->reg.reg_beg = reg.reg_beg;
		p_data->reg.reg_end = reg.reg_end;
		p_data->reg.reg_opt = reg.reg_opt;

		LS_INS_BEFORE(_regInfoList, p_data);

//		printk(KERN_INFO "%016lx-%016lx %lx", p_data->reg.reg_beg, p_data->reg.reg_end, p_data->reg.reg_opt);
//		printk("\n");
	}
	else
	{
		addr = 0;
	}

	return addr;
}

static ssize_t lgdrv_read(struct file *fp, char *buf, size_t cnt, loff_t *pos)
{
	int				err;
	pid_t			pid;
	reg_info_t*		p_base = NULL;
	reg_info_t*		p_data = NULL;
	region_t		reg;
	unsigned long	addr;

	if ((err = copy_from_user(&reg, buf, cnt)) < 0)
		return err;

	addr   = reg.reg_beg;
	pid    = get_curr_pid();
	p_base = LS_BASE(reg_info_t, _regInfoList);
	p_data = p_base->next;

	while(p_data != p_base)
	{
		if(p_data->pid == pid && p_data->addr == addr)
			break;
		p_data = p_data->next;
	}

	if(p_data != NULL && p_data != p_base)
	{
		reg.reg_beg = p_data->reg.reg_beg;
		reg.reg_end = p_data->reg.reg_end;
		reg.reg_opt = p_data->reg.reg_opt;
		LS_REMQUE(p_data);
		kfree(p_data);
	}
	else
	{
		return 0;
	}

	if(cnt != sizeof(region_t))
		return 0;

	if ((err = copy_to_user(buf, &reg, cnt)) < 0)
		return err;

	return cnt;
}

static ssize_t lgdrv_write(struct file *fp, const char *buf, size_t cnt, loff_t *pos)
{
	return 0;
}

static long lgdrv_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
{
	long		ret	= 0;
	ssize_t		size;


	mutex_lock(&lgioctl_lock);
	size = _IOC_SIZE(cmd);
	switch(cmd)
	{
		case LG_READ_CONTEXT:
			{
				volatile struct task_struct*	cp = NULL;
				struct task_struct*				p  = NULL;
				struct task_struct*				t  = NULL;
				context_t						context;

				if (copy_from_user(&context, (void*)arg, size))
				{
					ret = -EFAULT;
					goto _OUT_IOCTL;
				}

				rcu_read_lock();
				//printk(KERN_INFO "input pid:%d\n", context.pid);
				for_each_process_thread(p, t)
				{
					pid_t pid = task_pid_vnr(t);

					if(pid == (pid_t)context.pid)
					{
						printk(KERN_INFO "%d...%d", pid, context.pid);
						if(cp == NULL)
							cp = t;
					}
				}

				printk(KERN_INFO "pid:%d\n", cp->pid);
				if(cp != NULL)
					ret = build_context_frame(&context, (struct task_struct**)&cp);

				if(ret == (long)size)
				{
					if(copy_to_user((void*)arg, &context, size))
					{
						ret = -EFAULT;
						rcu_read_unlock();
						goto _OUT_IOCTL;
					}
				}
				else
				{
					ret = -EFAULT;
				}
				rcu_read_unlock();
			}
			break;
		default:
			ret = -EFAULT;
	}

_OUT_IOCTL:
	mutex_unlock(&lgioctl_lock);

	return ret;
}

static int lgdrv_release(struct inode *node, struct file *fp)
{
	printk(KERN_INFO "lg driver close called\n");
	module_put(THIS_MODULE);
	return 0;
}

/*-----------------------------------------------------------------------------
 *	lgdrv device init & exit logic
------------------------------------------------------------------------------*/
static struct file_operations lgdrv_fops =
{
	.owner          = THIS_MODULE,
	.open           = lgdrv_open,
	.read           = lgdrv_read,
	.write          = lgdrv_write,
	.unlocked_ioctl = lgdrv_ioctl,
	.llseek         = lgdrv_llseek,
	.release        = lgdrv_release,
};

static char* _lgdrv_devnode(struct device *dev, umode_t *mode)
{
	if(mode)
		*mode = 0666;

	return NULL;
}

static int __init lgdrv_init(void)
{
	int		ret = 0;

	if((ret = alloc_chrdev_region(&lgdrv_id, 0, 1, LGDRV_NAME)) != 0)
	{
    	printk("lgdrv: alloc_chrdev_region() error!\n");
		return ret;
	}

	cdev_init(&lgdrv_cdev, &lgdrv_fops);
	lgdrv_cdev.owner = THIS_MODULE;
	lgdrv_cdev.ops   = &lgdrv_fops;

	if((ret = cdev_add(&lgdrv_cdev, lgdrv_id, 1)) != 0)
	{
    	printk("lgdrv: cdev_add() error!\n");
		goto __OUT_ERR_CHRDEV_REGION;
	}

	lgdrv_class = class_create(THIS_MODULE, LGDRV_NAME);
	if( IS_ERR(lgdrv_class) )
	{
		ret = PTR_ERR(lgdrv_class);

		printk("lgdrv: class_create() error(%d)!\n", ret);
		goto __OUT_ERR_CDEV_DEL;
	}
	lgdrv_class->devnode = _lgdrv_devnode;

	lgdrv_dev = device_create(lgdrv_class, NULL, lgdrv_id, NULL, LGDRV_NAME);
	if( IS_ERR(lgdrv_dev) )
	{
		ret = PTR_ERR(lgdrv_dev);

		printk("lgdrv: device_create() error(%d)!\n", ret);
		goto __OUT_ERR_CLASS_DEST;
	}

    printk(KERN_INFO "lg driver init!\n");
	/* Linked List init */
	LS_INIT(_regInfoList);
	ret = 0;
	goto __OUT_OK;

__OUT_ERR_CLASS_DEST:
	class_destroy(lgdrv_class);
__OUT_ERR_CDEV_DEL:
	cdev_del(&lgdrv_cdev);
__OUT_ERR_CHRDEV_REGION:
	unregister_chrdev_region( lgdrv_id, 1 );
__OUT_OK:
    return ret;    // Non-zero return means that the module couldn't be loaded.
}

static void __exit lgdrv_exit(void)
{
	device_destroy(lgdrv_class, lgdrv_id);
	class_destroy(lgdrv_class);
	cdev_del(&lgdrv_cdev);
	unregister_chrdev_region(lgdrv_id, 1);
    printk(KERN_INFO "lg driver exit!\n");
}

module_init(lgdrv_init);
module_exit(lgdrv_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Baekwon Choi");
MODULE_DESCRIPTION("A Simple Hello World module");
