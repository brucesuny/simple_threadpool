#include "libthreadpool.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void *test_func(void *params)
{
	int *info = (int *)params;
	printf("thread 0x%x, tastnum %d\n", pthread_self(),  *info);

	return 0;
}



int main(int argc, char *argv[])
{
	struct threadpool_t *pool = NULL;
	const int thread_num =5;

	pool = sme_threadpool_init(thread_num);
	if (!pool)
	{
		printf("can not init pool\n");
		return -1;
	}
	
	printf("sme_threadpool_init finish!!\n");

	int *arg = (int *)malloc(sizeof(int) * 10);
	for (int i = 0; i < 10; i++) {
		arg[i] = i;
		sme_threadpool_add_job(pool, test_func, (void*)&arg[i]);
	}

	sleep(5);

	sme_threadpool_destory(pool);

	return 0;
}
