#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <iostream>

using namespace std;

typedef int (*printf_proc_t)(const char*,...);

extern "C" int					xlibc_setPrintStat(int);
extern "C" printf_proc_t		orgPrintf;
extern "C" void*				xlibc_getHookFunc(const char* p_name);
extern "C" void*				xlibc_getOrigFunc(const char* p_name);
extern "C" void					hexdump(const char* name, void* vcp, int size);
extern "C" uint64_t				findSymByName(const char *symName);
extern "C" uint64_t				findSymByAddr(uint64_t addr, char **pSymName);
extern "C" int					dsmInst(uint64_t* pInst, uint64_t address, void*);
extern "C" void					stack_trace (void);
extern "C" int					xlibc_setCoutStat(int _new);

class Test
{
public:
	Test()  {printf("this is my printf for class: %s\n", __func__);}
	~Test() {}
private:
	int n;
};

//extern "C"
void my_task1(void* arg)
{
	while (1)
	{
		usleep(2000000);
//		printf("[%x] %s run\n",(unsigned int)pthread_self(), __func__);
	}
}

//extern "C"
void my_task2(void* arg)
{
	while (1)
	{
		usleep(3000000);
//		printf("[%x] %s run\n",(unsigned int)pthread_self(), __func__);
	}
}

//extern "C"
void my_task3(void* arg)
{
	while (1)
	{
		usleep(1000000);
//		printf("[%x] %s run\n",(unsigned int)pthread_self(), __func__);
	}
}

//extern "C"
void my_task4(void* arg)
{
	while (1)
	{
		usleep(10000000);
//		printf("[%x] %s run\n",(unsigned int)pthread_self(), __func__);
	}
}

void* pTask[] = {	(void*)my_task1,
					(void*)my_task2,
					(void*)my_task3,
					(void*)my_task4
				};


Test t;
int main (void)
{
	int			loop;
	int			svd_lvl = xlibc_setPrintStat(2);

	puts("this is 1st example\n");
	printf("this is printf(%p)\n", printf);
	puts("this is puts\n");
	orgPrintf("this is orgPrintf(%p)\n", orgPrintf);

	cout << "******CPP cout :this is cout ostream  cout" << endl;

	svd_lvl = xlibc_setPrintStat(svd_lvl);
	printf("vprintf: h.%p o.%p\n", xlibc_getHookFunc("vprintf"), xlibc_getOrigFunc("vprintf"));

	hexdump("class Test t", (void*)main, 320);


	for (loop = 0; loop < 10; loop++)
	{
		if ((loop)%2)
			xlibc_setPrintStat(0);
		else
			xlibc_setPrintStat(2);
		cout << "cout print out test: " << loop << endl;
	}

	xlibc_setPrintStat(2);
	if(0)
	{
		uint64_t	addr;
		char*		name;
		addr = findSymByName((const char*)"my_task1");	printf("my_task1: %lx\n", addr);
		findSymByAddr(addr, &name); printf("%lx: %s\n", addr, name);

		addr = findSymByName((const char*)"my_task2");	printf("my_task2: %lx\n", addr);
		findSymByAddr(addr, &name); printf("%lx: %s\n", addr, name);

		addr = findSymByName((const char*)"my_task3");	printf("my_task3: %lx\n", addr);
		findSymByAddr(addr, &name); printf("%lx: %s\n", addr, name);

		addr = findSymByName((const char*)"my_task4");	printf("my_task4: %lx\n", addr);
		findSymByAddr(addr, &name); printf("%lx: %s\n", addr, name);
	}
	for(loop = 0; loop < 10; loop++)
	{
		pthread_attr_t	attr;
		pthread_t		tid;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&tid, &attr, (void*(*)(void*))pTask[loop%4], (void*)&loop);
		pthread_attr_destroy(&attr);

	}

	int		  inc;
	uint64_t  addr = (uint64_t )stack_trace;
	uint64_t* inst = (uint64_t*)stack_trace;

	for(int i = 0; i < 10; i++)
	{
		inc = dsmInst((uint64_t*)inst, (uint64_t)addr, (void*)printf);
		addr += inc;
		inst  = (uint64_t*)addr;
		if(inc == 0)
		{
			break;
		}
	}

	while (1)
	{
		usleep(1000);
	}
	return 0;
}
