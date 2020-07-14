#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>

typedef int (*printf_proc_t)(const char*,...);

extern int xlibc_setPrintStat(int);
extern printf_proc_t		orgPrintf;
extern void* xlibc_getHookFunc(const char* p_name);
extern void* xlibc_getOrigFunc(const char* p_name);
extern void hexdump(const char* name, void* vcp, int size);

void my_task(void)
{
	while (1)
	{
		usleep(1000);
	}
}

int main (void)
{
	int svd_lvl = xlibc_setPrintStat(1);

	puts("this is 1st example\n");
	printf("printf: puts %p %p\n", puts, printf);

	puts("this is 2nd example\n");


	printf("printf: puts %p %p\n", puts, printf);
	svd_lvl = xlibc_setPrintStat(svd_lvl);


	printf("vprintf: h.%p o.%p\n", xlibc_getHookFunc("vprintf"), xlibc_getOrigFunc("vprintf"));

	hexdump("hex", (void*)main, 320);
	{
		pthread_attr_t	attr;
		pthread_t		tid;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&tid, &attr, (void*(*)(void*))my_task, NULL);
		pthread_attr_destroy(&attr);

	}


	while (1)
	{
		usleep(1000);
	}
	return 0;
}
