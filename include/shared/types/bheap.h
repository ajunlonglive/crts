#ifndef SHARED_TYPES_BHEAP_H
#define SHARED_TYPES_BHEAP_H

#include <stdint.h>

#include "shared/types/darr.h"

/* the backing darr for a "bheap" darr must have a data layout where the first
 * 32 bits are a uin32_t, which represents the value to be sorted on.  Right
 * now this is just min heap
 */

void bheap_heapify(struct darr *bh);
void *bheap_peek(struct darr *bh);
void bheap_pop(struct darr *bh);
void bheap_push(struct darr *bh, const void *e);
void bheap_up_heapify(uint8_t *bh, uint32_t isize, uint32_t len, uint32_t i);
void bheap_down_heapify(uint8_t *bh, uint32_t isize, uint32_t len, uint32_t i);
#endif
