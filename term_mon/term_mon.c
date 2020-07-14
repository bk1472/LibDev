#include	<sys/mman.h>
#include	<sys/stat.h>        /* For mode constants */
#include	<fcntl.h>           /* For O_* constants */
#include	<sys/types.h>
#include	<termios.h>
#include	<unistd.h>

#include	<errno.h>

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>

#define MAGIC_SHM		0x7BED8669
#define SZ_SHM_BUF		(20)

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


static char		term_name[3][TERM_NM_MAX];


static int build_termname(void)
{
	int		iter;

	for (iter = TERM__IN; iter <=TERM_ERR; iter++)
	{
		if (ttyname_r(iter, &term_name[iter][0], TERM_NM_MAX-1) != 0)
		{
			printf("* ERROR: open %d file\n", iter);
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

#if 0
static void term_setmode(int flag, int mode)
{
	struct termios termAttr;
	tcgetattr(0, &termAttr);
	termAttr.c_cc[VERASE]  =  0x008; /* Backspace      */
	termAttr.c_lflag      &= ~0xa00; /* Backspace Echo */

	if (mode & ECHO)
	{
		if (flag & ECHO) termAttr.c_lflag |=  ECHO;
		else             termAttr.c_lflag &= ~ECHO;
	}
	if (mode & ICANON)
	{
		if (flag & ICANON) termAttr.c_lflag |=  ICANON;
		else               termAttr.c_lflag &= ~ICANON;
	}

	tcsetattr(0, TCSANOW, &termAttr);
	return;
}
#endif

int main (int argc, char* argv[])
{
	int			shm_fd     = -1;
	int			n_read     = 0;
	size_t		n_input    = 128;
	int			idx;

	char*		p_input    = NULL;
	char*		shm_ptr    = NULL;
	SHM_ATTR_t*	p_shm_attr = NULL;

	if(build_termname() != 0)
		return -1;

	for ( idx = TERM__IN; idx <= TERM_ERR; idx++ )
	{
		char* s = get_termname(idx);
		if(s) printf("[%d]term_name: %s\n", idx, s);
	}

	p_input = (char *)malloc(n_input);
	if ((shm_fd = shm_open(NAME_DBG_SHM, O_RDWR, 0)) == -1)
	{
		printf("* ERROR: /dev/shm%s cat't be found!\n", NAME_DBG_SHM);
		exit(EXIT_FAILURE);
	}

	shm_ptr = (char *)mmap(NULL, SZ_DBG_SHM_SEG, PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (shm_ptr == MAP_FAILED)
	{
		printf("* ERROR: shared memory mmap can't open!\n");
		exit(EXIT_FAILURE);
	}
	p_shm_attr = (SHM_ATTR_t*)shm_ptr;

	printf(" ============================\n");
	printf("* SHM Name  : /dev/shm%s\n", NAME_DBG_SHM);
	printf(" ----------------------------\n");
	printf("  magic     :0x%08x\n", p_shm_attr->magic);
	printf("  shm id    :%d\n",     p_shm_attr->shm_id);
	printf("  shm name  :%s\n",     p_shm_attr->p_shm_name);
	printf("  stm name  :%s\n",     p_shm_attr->s_term_nm);
	printf("  ctm name  :%s\n",     p_shm_attr->c_term_nm);

	while (1)
	{
		printf("\n>> ");

		if ((n_read = (int)getline(&p_input, &n_input, stdin)) == -1)
			return -1;

		switch (p_input[0])
		{
			case '.':
				goto _END;
				break;
			case '*':
			{
				int null_fd;
				printf("  ctm name  :%s -> ",     p_shm_attr->c_term_nm);
				strncpy(p_shm_attr->c_term_nm, get_termname(1), TERM_NM_MAX);
				printf(" %s\n",     p_shm_attr->c_term_nm);
				null_fd = open("/dev/null", O_RDWR);
				if(null_fd >= 0)
				{
					//term_setmode(0, ICANON|ECHO);
					close(0);close(1);close(2);
					dup2(null_fd, 0);
					dup2(null_fd, 1);
					dup2(null_fd, 2);
					sleep(0xFFFFFFFF);
				}
			} break;
			default:
				break;
		}
	}

_END:
	free(p_input);

	return 0;
}
