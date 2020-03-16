#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "shared/net/msg_queue.h"
#include "shared/types/iterator.h"
#include "shared/util/log.h"

enum msginfo_states {
	mis_new = 1 << 0,
	mis_full = 1 << 1,
};

struct msginfo {
	msg_seq_t seq;
	uint32_t cooldown;
	uint32_t age;
	cx_bits_t send_to;
	enum msg_flags flags;
	uint8_t state;
};

struct msg_queue {
	char *msgs;
	size_t msgsize;
	msg_seq_t seq;
	size_t len;
	bool full;
};

static msg_seq_t
next_seq(struct msg_queue *q)
{
	return q->seq = (q->seq + 1) & (MSG_ID_LIM - 1);
}

static void
del_msg(struct msg_queue *q, struct msginfo *mi)
{
	mi->state = mi->flags = 0;
	if (q->len > 0) {
		q->full = false;
		--q->len;
	}

}

struct msg_queue *
msgq_init(size_t msgsize)
{
	struct msg_queue *mq = calloc(1, sizeof(struct msg_queue));

	mq->msgs = calloc(MSG_ID_LIM, sizeof(struct msginfo) + msgsize);
	mq->msgsize = msgsize;

	return mq;
}

static void *
msgq_get_mi(struct msg_queue *q, msg_seq_t seq)
{
	VBASSERT(seq < MSG_ID_LIM, "msgq accessing invalid msg %d", seq);

	return q->msgs + (seq * (sizeof(struct msginfo) + q->msgsize));
}

static void *
msgq_get_data(struct msg_queue *q, msg_seq_t seq)
{
	VBASSERT(seq < MSG_ID_LIM, "msgq accessing invalid msg %d", seq);

	return q->msgs + (seq * (sizeof(struct msginfo) + q->msgsize)) + sizeof(struct msginfo);
}

void *
msgq_add(struct msg_queue *q, cx_bits_t send_to, enum msg_flags flags)
{
	struct msginfo *mi, hdr = { next_seq(q), 0, 0, send_to, flags, mis_new | mis_full };

	msg_seq_t oseq = hdr.seq;

	mi = msgq_get_mi(q, hdr.seq);

	if (send_to == 0) {
		L("not sending to anyone, skipping");
		return NULL;
	} else if (q->full && hdr.flags & msgf_drop_if_full) {
		L("queue is full, skipping");
		return NULL;
	} else if (q->full && hdr.flags & msgf_must_send) {
		while (mi->state & mis_full && mi->flags & msgf_must_send) {
			if ((hdr.seq = next_seq(q)) == oseq) {
				L("all full of important messages ?");
				return NULL;
			}
			L("checking %d ", hdr.seq);
			mi = msgq_get_mi(q, hdr.seq);
		}
	}

	memcpy(mi, &hdr, sizeof(struct msginfo));

	if (++q->len == MSG_ID_LIM) {
		q->full = true;
	}

	return msgq_get_data(q, mi->seq);
}

struct msgq_ack_ctx {
	struct msg_queue *q;
	cx_bits_t acker;
};

enum iteration_result
msgq_ack_iter(void *_ctx, msg_seq_t ackd)
{
	struct msginfo *mh;
	struct msgq_ack_ctx *ctx = _ctx;

	mh = msgq_get_mi(ctx->q, ackd);

	if (!(mh->state & mis_full)) {
		return ir_cont;
	}

	mh->send_to &= ~ctx->acker;

	if (!mh->send_to) {
		del_msg(ctx->q, mh);
	}

	return ir_cont;
}

void
msgq_ack(struct msg_queue *q, struct acks *a, cx_bits_t acker)
{
	struct msgq_ack_ctx ctx = { q, acker };

	ack_iter(a, &ctx, msgq_ack_iter);
}

void
msgq_send_all(struct msg_queue *q, void *ctx, msgq_send_all_iter sendf)
{
	msg_seq_t i;
	struct msginfo *mi;

	for (i = 0; i < MSG_ID_LIM; ++i) {
		if (!((mi = msgq_get_mi(q, i))->state & mis_full)) {
			continue;
		}

		if (mi->send_to && mi->cooldown == 0) {
			if (mi->state & mis_new) {
				mi->state &= ~mis_new;
			} else {
				L("resending %x", i);
			}

			mi->cooldown = MSG_RESEND_AFTER;
			sendf(ctx, mi->send_to, mi->seq, mi->flags, msgq_get_data(q, i));

			if ((mi->flags & msgf_forget) || ++mi->age >= MSG_DESTROY_AFTER) {
				del_msg(q, mi);
			}
		} else if (mi->cooldown > 0) {
			--mi->cooldown;
		}
	}
}
