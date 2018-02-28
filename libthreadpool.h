#ifndef SME_THREADPOOL_H
#define SME_THREADPOOL_H
#include <pthread.h>

typedef struct job_t
{
	void* (*callback_function)(void* params);
	void *params;
	void *ret;

	struct job_t *next;
}job_t;

struct threadpool_t
{
	int exit;
	int threads_max_num;
	job_t *queue_head;
	int  queue_cur_num;

	pthread_t *thread_id;
	pthread_mutex_t mutex;
	pthread_cond_t cv_que_ready;
};

struct threadpool_t* sme_threadpool_init(int threads_max_num);

int sme_threadpool_add_job(struct threadpool_t *threadpool, void* (*callback_func)(void *params), void *params);

int sme_threadpool_destory(struct threadpool_t *threadpool);

#endif
