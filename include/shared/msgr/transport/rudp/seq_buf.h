#ifndef SHARED_MSGR_TRANSPORT_RUDP_SEQ_BUF_H
#define SHARED_MSGR_TRANSPORT_RUDP_SEQ_BUF_H

#include <stdbool.h>
#include <stdint.h>

#include "shared/types/darr.h"

struct seq_buf {
	uint16_t head;
	struct darr dat;
};

void seq_buf_init(struct seq_buf *sb, uint32_t isize);
void seq_buf_destroy(struct seq_buf *sb);
void *seq_buf_get(struct seq_buf *sb, uint16_t seq);
void *seq_buf_insert(struct seq_buf *sb, uint16_t seq);
uint32_t seq_buf_gen_ack_bits(struct seq_buf *sb);
#endif
