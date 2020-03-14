#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "shared/net/msg_queue.h"
#include "shared/types/darr.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"

struct msg_queue {
	struct hdarr *msgs;
	struct darr *to_flush;
	size_t msgsize;
	msg_seq_t seq;
};

struct msginfo {
	msg_seq_t seq;
	uint32_t cooldown;
	uint32_t age;
	cx_bits_t send_to;
	bool new;
};

static void *
msgq_key_getter(void *_mi)
{
	struct msginfo *mi = _mi;

	return &mi->seq;
}

static msg_seq_t
next_seq(struct msg_queue *q)
{
	return q->seq = (q->seq + 1) & (MSG_ID_LIM - 1);
}

struct msg_queue *
msgq_init(size_t msgsize)
{
	struct msg_queue *mq = calloc(1, sizeof(struct msg_queue));

	mq->msgs = hdarr_init(MSG_ID_LIM, sizeof(msg_seq_t),
		sizeof(struct msginfo) + msgsize, msgq_key_getter);
	mq->to_flush = darr_init(sizeof(msg_seq_t));

	return mq;
}

void *
msgq_add(struct msg_queue *q, cx_bits_t send_to)
{
	struct msginfo hdr = { next_seq(q), 0, 0, send_to, true };

	if (hdarr_get(q->msgs, &hdr.seq) != NULL) {
		L("overwriting msg %x", hdr.seq);
	}

	size_t i = hdarr_set(q->msgs, &hdr.seq, &hdr);

	return darr_point_at(hdarr_darr(q->msgs), i) + sizeof(struct msginfo);
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

	if ((mh = hdarr_get(ctx->q->msgs, &ackd)) == NULL) {
		return ir_cont;
	}

	mh->send_to &= ~ctx->acker;

	if (!mh->send_to) {
		darr_push(ctx->q->to_flush, &ackd);
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
msgq_send_all(struct msg_queue *q, void *ctx, msgq_send_all_iter send)
{
	size_t i, len = hdarr_len(q->msgs);
	msg_seq_t newseq;
	char *mem;
	struct msginfo *mh;

	for (i = 0; i < len; ++i) {
		mem = darr_point_at(hdarr_darr(q->msgs), i);
		mh = (struct msginfo *)mem;

		if (mh->send_to && mh->cooldown == 0) {
			mem += sizeof(struct msginfo);

			if (mh->new) {
				mh->new = false;
			} else {
				newseq = next_seq(q);
				hdarr_reset(q->msgs, &mh->seq, &newseq);
				L("resending (old seq: %x, new seq: %x)", mh->seq, newseq);
				mh->seq = newseq;
			}

			mh->cooldown = MSG_RESEND_AFTER;
			send(ctx, mh->send_to, mh->seq, mem);

			if (++mh->age >= MSG_DESTROY_AFTER) {
				darr_push(q->to_flush, &mh->seq);
			}
		} else if (mh->cooldown > 0) {
			--mh->cooldown;
		}
	}
}

static enum iteration_result
flush_msg(void *_q, void *_seq)
{
	msg_seq_t *seq = _seq;
	struct msg_queue *q = _q;

	hdarr_del_p(q->msgs, seq, true);

	return ir_cont;
}

void
msgq_flush(struct msg_queue *q)
{
	darr_for_each(q->to_flush, q, flush_msg);
	darr_clear(q->to_flush);
}
