#define _POSIX_C_SOURCE 201900L

#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "types/queue.h"
#include "util/log.h"

#define QUEUE_CAP 255;

struct queue *queue_init(void)
{
	struct queue *q = malloc(sizeof(struct queue));
	pthread_mutexattr_t mattr;

	q->cap = QUEUE_CAP;
	q->head = 0;
	q->tail = 0;
	q->data = calloc(q->cap, sizeof(void *));

	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setrobust(&mattr, PTHREAD_MUTEX_ROBUST);
	pthread_mutex_init(&q->mutex, &mattr);

	pthread_cond_init(&q->not_empty, NULL);
	pthread_cond_init(&q->not_full, NULL);
	q->waiting_pop = 0;
	q->waiting_push = 0;

	L("initialized queue of size %ld", (long)q->cap);

	return q;
};

static size_t rt_cc(size_t start, size_t circ)
{
	if (start <= 0)
		return circ - 1;
	else
		return start - 1;
}

static size_t q_len(struct queue *q)
{
	if (q->head < q->tail)
		return q->head + (q->cap - q->tail);
	else if (q->head > q->tail)
		return q->head - q->tail;
	else
		return 0;
}

void queue_push(struct queue *q, void *data)
{
	int mr;

	if ((mr = pthread_mutex_lock(&q->mutex)) != 0)
		L("error locking mutex: %s", strerror(mr));

	if (q_len(q) >= q->cap - 1) {
		q->waiting_push = 1;
		pthread_cond_wait(&q->not_full, &q->mutex);
		q->waiting_push = 0;
	}

	q->data[q->tail] = data;
	q->tail = rt_cc(q->tail, q->cap);

	if (q->waiting_pop > 0)
		pthread_cond_signal(&q->not_empty);

	if ((mr = pthread_mutex_unlock(&q->mutex)) != 0)
		L("error unlocking mutex: %s", strerror(mr));
};

void *queue_pop(struct queue *q, int block)
{
	void *r = NULL;
	int mr;
	int l;

	if ((mr = pthread_mutex_lock(&q->mutex)) != 0)
		L("error locking mutex: %s", strerror(mr));

	l = q_len(q);
	if (l <= 0 && block == 1) {
		q->waiting_pop = 1;
		pthread_cond_wait(&q->not_empty, &q->mutex);
		q->waiting_pop = 0;
	} else if (l <= 0 && block == 0) {
		goto unlock;
	}

	r = q->data[q->head];
	q->head = rt_cc(q->head, q->cap);

	if (q->waiting_push > 0)
		pthread_cond_signal(&q->not_full);
unlock:
	if ((mr = pthread_mutex_unlock(&q->mutex)) != 0)
		L("error unlocking mutex: %s", strerror(mr));

	return r;
};
