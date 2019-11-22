#define _POSIX_C_SOURCE 201900L

#include <string.h>
#include "queue.h"
#include "log.h"
#include <pthread.h>
#include <stdlib.h>

#define QUEUE_CAP 16;

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
	L("initialized queue of size %d", q->cap);

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

	if (q_len(q) >= q->cap - 1)
		goto unlock;

	q->tail = rt_cc(q->tail, q->cap);
	q->data[q->tail] = data;

unlock:
	if ((mr = pthread_mutex_unlock(&q->mutex)) != 0)
		L("error unlocking mutex: %s", strerror(mr));

	//L("pushed to q@%p, queue %d->%d | len: %d", q, q->tail, q->head, q_len(q));
};

void *queue_pop(struct queue *q)
{
	void *r = NULL;
	int mr;

	if ((mr = pthread_mutex_lock(&q->mutex)) != 0)
		L("error locking mutex: %s", strerror(mr));

	if (q_len(q) <= 0)
		goto unlock;

	r = q->data[q->head];
	q->head = rt_cc(q->head, q->cap);

unlock:
	if ((mr = pthread_mutex_unlock(&q->mutex)) != 0)
		L("error unlocking mutex: %s", strerror(mr));

	return r;
};
