#include "libthreadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void* threadpool_do_function(void *params)
{
	struct threadpool_t *pool = (struct threadpool_t *)params;
	job_t *pjob = NULL;

	while (1)
	{
		pthread_mutex_lock(&(pool->mutex));
		while (!pool->queue_cur_num && !pool->exit)
			pthread_cond_wait(&(pool->cv_que_ready), &(pool->mutex));
		
		if (pool->exit)
		{
			pthread_mutex_unlock(&(pool->mutex));
			printf("the thread 0x%x exit\n", pthread_self());
			pthread_exit(NULL);
		}
		
		pool->queue_cur_num--;
		pjob = pool->queue_head;
		pool->queue_head = pool->queue_head->next;

		pthread_mutex_unlock(&(pool->mutex));

		pjob->ret = (*(pjob->callback_function))(pjob->params);
		free(pjob);
		pjob = NULL;
	}

	return NULL;
}

struct threadpool_t* sme_threadpool_init(int threads_max_num)
{
	struct threadpool_t *pool = NULL;

	do {
		pool =(struct threadpool_t *) malloc(sizeof(struct threadpool_t));
		if (!pool) {
			printf("can not malloc pool\n");
			break;
		}
		pool->exit = 0;
		pool->queue_cur_num = 0;
		pool->threads_max_num = threads_max_num;

		if (pthread_mutex_init(&(pool->mutex), NULL)) {
			printf("can not mutex init\n");
			break;
		}

		if (pthread_cond_init(&(pool->cv_que_ready), NULL)) {
			printf("can not cond init\n");
			break;
		}

		pool->thread_id = (pthread_t *)malloc(sizeof(pthread_t) * threads_max_num);
		if (!pool->thread_id) {
			printf("can not malloc thread_id\n");
			break;
		}

		for (int i = 0; i < threads_max_num; i++) {
			pthread_create(&(pool->thread_id[i]), NULL, threadpool_do_function, (void *)pool);
			printf("thread_id[%d] %x\n", i, pool->thread_id[i]);
		}
	
		return pool;
	} while (0);

	return NULL;
}

int sme_threadpool_add_job( struct threadpool_t *threadpool, void* (*callback_func)(void *params), void *params)
{
	struct threadpool_t *pool = NULL;
	job_t *pjob = NULL;
	job_t *p_mem_queue = NULL;

	assert(threadpool != NULL);
	assert(callback_func != NULL);

	pool = threadpool;
	pjob = (job_t *)malloc(sizeof(job_t));
	if (!pjob)
	{
		printf("can not malloc job\n");
	}

	pjob->callback_function = callback_func;
	pjob->params = params;
	pjob->next = NULL;

	pthread_mutex_lock(&(pool->mutex));

	p_mem_queue = pool->queue_head;
	if (p_mem_queue != NULL)
	{
		while (p_mem_queue->next != NULL)
			p_mem_queue = p_mem_queue->next;
		p_mem_queue->next = pjob;
	}
	else {
		pool->queue_head = pjob;
	}
	pool->queue_cur_num++;

	pthread_cond_signal(&(pool->cv_que_ready));
	pthread_mutex_unlock(&(pool->mutex));

	return 0;
}

int sme_threadpool_destory(struct threadpool_t *threadpool)
{
	struct threadpool_t *pool = threadpool;
	if (pool->exit)
		return -1;

	pool->exit = 1;

	pthread_cond_broadcast(&(pool->cv_que_ready));

	for (int i = 0; i < pool->threads_max_num; i++)
		pthread_join(pool->thread_id[i], NULL);

	free(pool->thread_id);

	job_t *job = NULL;
	while (pool->queue_head != NULL)
	{
		job = pool->queue_head;
		pool->queue_head = pool->queue_head->next;
		free(job);
	}

	pthread_mutex_destroy(&(pool->mutex));
	pthread_cond_destroy(&(pool->cv_que_ready));

	free(pool);
	pool = NULL;
	
	return 0;
}
