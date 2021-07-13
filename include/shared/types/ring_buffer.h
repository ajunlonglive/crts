#ifndef SHARED_TYPES_RING_BUFFER_H
#define SHARED_TYPES_RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

struct ring_buffer {
	uint8_t *buf;
	uint32_t item_size, mask;
	_Atomic uint32_t read, write;
};


void ring_buffer_init(struct ring_buffer *rb, uint32_t item_size, uint32_t len);
void ring_buffer_deinit(struct ring_buffer *rb);
void *ring_buffer_pop(struct ring_buffer *rb);
bool ring_buffer_push(struct ring_buffer *rb, void *val);
#endif
