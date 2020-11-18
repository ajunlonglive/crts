#ifndef SHARED_NET_ACK_H
#define SHARED_NET_ACK_H

#include <stdbool.h>

#include "shared/net/defs.h"
#include "shared/types/hash.h"
#include "shared/types/iterator.h"

struct msg_queue;

#define ACK_CAP (sizeof(ack_t) * 8)

typedef uint32_t ack_t;

struct ack_group {
	msg_seq_t leader;
	ack_t acks;
};

typedef enum iteration_result (*ack_iter_func)(void *, msg_seq_t);

void ack_init(struct hash *hash);
void ack_set(struct hash *ags, msg_seq_t new);
void ack_clear_all(struct hash *ags);
bool ack_check(struct hash *ags, msg_seq_t id);
void ack_msgq(struct hash *ags, struct msg_queue *q, cx_bits_t acker);
#endif
