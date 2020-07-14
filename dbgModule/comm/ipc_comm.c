#include	"ipc_comm.h"
#include	"xlibc_output.h"
#include	"xlibc_termios.h"
#include	"xlibc_util.h"
#include	"sig_handle.h"
#include	"thread.h"


#define MAGIC_SHM		0x7BED8669
#define SZ_SHM_BUF		(20)

/**
 * Definitions of IPC resources
 */
#define	ROOT_PATH		"/tmp/dbgModule"

#define	NAME_DBG_SHM	"/dbgSHM"
#define	SZ_DBG_SHM_SEG	(8192)
#define NUL_TERM		"non"

#define	TERM_NM_MAX		(20)
#define TERM__IN		(0)
#define TERM_OUT		(TERM__IN + 1)
#define TERM_ERR		(TERM__IN + 2)

typedef struct _shm_attr_
{
	/* Shared memory info. */
	int		magic;
	int		shm_id;
	char	p_shm_name[SZ_SHM_BUF];

	/* Server Pgm info. */
	char	s_term_nm[TERM_NM_MAX];

	/* Client Pgm info. */
	char	c_term_nm[TERM_NM_MAX];
} SHM_ATTR_t;


static char			term_name[3][TERM_NM_MAX];
static SHM_ATTR_t*	p_shmAttr = NULL;


static int build_termname(void)
{
	int		iter;

	for (iter = TERM__IN; iter <=TERM_ERR; iter++)
	{
		if (ttyname_r(iter, &term_name[iter][0], TERM_NM_MAX-1) != 0)
		{
			dbgprint("^R^ERROR: open %d file", iter);
			term_name[iter][0] = 0;
			return -1;
		}
		term_name[iter][TERM_NM_MAX-1] = '\0';
	}

	return 0;
}

static char* get_termname(int index)
{
	if (index < TERM__IN || index > TERM_ERR)
		return NULL;

	return &term_name[index][0];
}

static void* open_shm(char* shm_nm, size_t sz)
{
	int		shm_fd  = -1;
	char*	shm_ptr = NULL;
	int*	p_head  = NULL;


	if (sz > SZ_DBG_SHM_SEG)
		return NULL;

	shm_fd = shm_open(shm_nm, O_RDWR | O_CREAT | O_EXCL, 0660);

	if (shm_fd > 0)
	{
		if (ftruncate(shm_fd, SZ_DBG_SHM_SEG) == -1)
			return NULL;
	}
	else
	{
		if (errno != EEXIST)
			return NULL;

		if ((shm_fd = shm_open(shm_nm, O_RDWR, 0)) == -1)
			return NULL;
	}

	shm_ptr = (char*)mmap(NULL, SZ_DBG_SHM_SEG, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

	if (shm_ptr == MAP_FAILED)
		return NULL;

	/**
	 * Fill SHM header
	 */
	p_head    = (int*)shm_ptr;
	p_head[0] = MAGIC_SHM;
	p_head[1] = shm_fd;

	return (void*)p_head;
}

static void _sigint_handler(int sigNo, siginfo_t* p_info, void* p_context)
{
	if (sigNo == SIGINT)
	{
		xlibc_term_setmode(ICANON|ECHO, ICANON|ECHO);
		close_shm();
		exit(0);
	}
}

void _ipc_cmd(void)
{
	int change_term = 0;

	while (1)
	{
		if (p_shmAttr == NULL)
			return;
		xlibc_suspend(50);

		if (change_term == 0)
		{
			if (    p_shmAttr->c_term_nm[0] != 'n'
				||  p_shmAttr->c_term_nm[1] != 'o'
				||  p_shmAttr->c_term_nm[2] != 'n'
			   )
			{
				int new_fd;
				/**
				 * (1) close stdin,out,err
				 * (2) reopen terminal to virtual terminal for term_mon
				 */
				new_fd = open(p_shmAttr->c_term_nm, O_RDWR);
				if(new_fd >=0)
				{
					dbgprint("^y^terminal change!");
					close(0);close(1);close(2);
					dup2(new_fd, 0);
					dup2(new_fd, 1);
					dup2(new_fd, 2);
					change_term = 1;
				}
			}
		}
	}
}

/**
 * Comm 초기화 setting
 */
void init_comm (void)
{
	pthread_attr_t	attr;
	pthread_t		tid;

	if (mkdir(ROOT_PATH, S_IRWXU) != 0)
	{
		if (errno != EEXIST)
			return;
	}
	if (build_termname() != 0)
		return;

	#if 0
	if (1)
	{
		int idx;
		for (idx = TERM__IN; idx <= TERM_ERR; idx++)
		{	char* s = get_termname(idx);
			if(s) dbgprint("^Y^[%d]term_name: %s", idx, s);
		}
	}
	#endif

	if ((p_shmAttr = open_shm(NAME_DBG_SHM, sizeof(SHM_ATTR_t))) == NULL)
		return;

	strncpy(p_shmAttr->p_shm_name, NAME_DBG_SHM,    SZ_SHM_BUF);
	strncpy(p_shmAttr->s_term_nm,  get_termname(0), TERM_NM_MAX);
	strncpy(p_shmAttr->c_term_nm,  NUL_TERM,        sizeof(NUL_TERM));
	#if 0
	dbgprint("^y^magic   :0x%08x", p_shmAttr->magic);
	dbgprint("^y^shm id  :%d",     p_shmAttr->shm_id);
	dbgprint("^y^shm name:%s",     p_shmAttr->p_shm_name);
	dbgprint("^y^stm name:%s",     p_shmAttr->s_term_nm);
	dbgprint("^y^ctm name:%s",     p_shmAttr->c_term_nm);
	#endif
	register_signal(SIGINT, _sigint_handler);


	if (0 != pthread_attr_init(&attr))
		return;
	if(0 == pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
	{
		pthread_create_ext("ipc_comm", &tid, &attr, (void*(*)(void*))_ipc_cmd, NULL);
	}
	pthread_attr_destroy(&attr);

	return;
}

void close_shm(void)
{
	if (p_shmAttr == NULL)
		return;

	munmap(p_shmAttr, SZ_DBG_SHM_SEG);
	p_shmAttr = NULL;
	shm_unlink(NAME_DBG_SHM);
}
