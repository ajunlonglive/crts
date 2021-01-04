#ifndef SHARED_TYPES_SACK_H
#define SHARED_TYPES_SACK_H

#include <stddef.h>
#include <stdint.h>

#include "shared/types/iterator.h"

typedef size_t (*sack_stuffing_func)(void *itm, uint8_t *buf, uint32_t blen);

struct sack {
	sack_stuffing_func stuff;
	uint8_t *mem;
	uint32_t len, cap, items;
	uint8_t hdr_len;
};

typedef enum del_iter_result (*sack_iter_cb)(void *ctx, void *hdr,
	void *itm, uint16_t itm_len);

void sack_init(struct sack *sk, uint8_t hdr_len, uint32_t cap, sack_stuffing_func stuff);
void sack_destroy(struct sack *sk);
void sack_stuff(struct sack *sk, void *hdr, void *itm);
void sack_iter(struct sack *sk, void *ctx, sack_iter_cb cb);
#endif
