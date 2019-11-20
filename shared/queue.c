#include "queue.h"
#include "log.h"
#include <stdlib.h>

#define QUEUE_CAP 16;

struct queue *queue_init(void)
{
	struct queue *q = malloc(sizeof(struct queue));

	q->cap = QUEUE_CAP;
	q->head = 0;
	q->tail = 0;
	q->data = calloc(q->cap, sizeof(void *));
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

static size_t rt_cw(size_t start, size_t circ)
{
	if (start >= circ - 1)
		return 0;
	else
		return start + 1;
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
	if (q_len(q) >= q->cap - 1)
		return;

	q->tail = rt_cc(q->tail, q->cap);
	q->data[q->tail] = data;
	L("pushed to q@%p, queue %d->%d | len: %d", q, q->tail, q->head, q_len(q));
};

void *queue_pop(struct queue *q)
{
	void *r;

	if (q_len(q) <= 0)
		return NULL;

	r = q->data[q->head];
	q->head = rt_cc(q->head, q->cap);

	return r;
};
