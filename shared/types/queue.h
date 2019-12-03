#ifndef __QUEUE_H
#define __QUEUE_H
#include <stdlib.h>
#include <pthread.h>

struct queue {
	size_t cap;
	size_t head;
	size_t tail;

	pthread_mutex_t mutex;
	pthread_cond_t not_full;
	pthread_cond_t not_empty;

	int waiting_push;
	int waiting_pop;

	void **data;
};

struct queue *queue_init(void);
void queue_push(struct queue *q, void *data);
void *queue_pop(struct queue *q, int block);
#endif
