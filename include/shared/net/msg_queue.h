#ifndef SHARED_NET_MSG_QUEUE_H
#define SHARED_NET_MSG_QUEUE_H

#include <stddef.h>

#include "shared/net/defs.h"
#include "shared/net/ack.h"

typedef void ((*msgq_send_all_iter)(void *, cx_bits_t, msg_seq_t, enum msg_flags, void *));

struct msg_queue;

struct msg_queue *msgq_init(size_t msgsize);
void *msgq_add(struct msg_queue *q, cx_bits_t send_to, enum msg_flags flags);
void msgq_ack(struct msg_queue *q, struct acks *a, cx_bits_t acker);
void msgq_send_all(struct msg_queue *q, void *ctx, msgq_send_all_iter);
#endif
