#ifndef SHARED_NET_ACK_H
#define SHARED_NET_ACK_H

#include <stdbool.h>

#include "shared/net/defs.h"
#include "shared/types/iterator.h"

#define ACK_BLOCK_LEN (sizeof(uint32_t) * 8)
#define ACK_BLOCKS (FRAME_LEN / ACK_BLOCK_LEN)

struct acks {
	uint32_t acks[ACK_BLOCKS];
};

typedef enum iteration_result (*ack_iter_func)(void *, msg_seq_t);

void ack_set(struct acks *a, msg_seq_t new);
void ack_clear_all(struct acks *a);
bool ack_check(struct acks *a, msg_seq_t id);
void ack_iter(struct acks *a, void *ctx, ack_iter_func func);
#endif
