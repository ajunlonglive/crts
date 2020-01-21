#define _POSIX_C_SOURCE 201900L

#include <string.h>
#include <stdlib.h>

#include "types/queue.h"
#include "util/log.h"

#define QUEUE_CAP 255;

struct queue *
queue_init(void)
{
	struct queue *q = malloc(sizeof(struct queue));

	q->cap = QUEUE_CAP;
	q->head = 0;
	q->tail = 0;
	q->data = calloc(q->cap, sizeof(void *));

	q->waiting_pop = 0;
	q->waiting_push = 0;

	L("initialized queue of size %ld", (long)q->cap);

	return q;
};

static size_t
rt_cc(size_t start, size_t circ)
{
	if (start <= 0) {
		return circ - 1;
	} else {
		return start - 1;
	}
}

static size_t
q_len(struct queue *q)
{
	if (q->head < q->tail) {
		return q->head + (q->cap - q->tail);
	} else if (q->head > q->tail) {
		return q->head - q->tail;
	} else {
		return 0;
	}
}

void
queue_push(struct queue *q, void *data)
{
	if (q_len(q) >= q->cap - 1) {
		L("can't push: queue full");
		return;
	}

	q->data[q->tail] = data;
	q->tail = rt_cc(q->tail, q->cap);
};

void *
queue_pop(struct queue *q)
{
	void *r = NULL;
	int l;

	l = q_len(q);
	if (l <= 0) {
		return NULL;
	}

	r = q->data[q->head];
	q->head = rt_cc(q->head, q->cap);

	return r;
};
