#ifndef __QUEUE_H
#define __QUEUE_H
#include <stdlib.h>

struct queue {
	size_t cap;
	size_t head;
	size_t tail;

	void **data;
};

struct queue *queue_init(void);
void queue_push(struct queue *q, void *data);
void *queue_pop(struct queue *q);
#endif
