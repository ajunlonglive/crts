#include "posix.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "shared/net/msg_queue.h"
#include "shared/serialize/message.h"
#include "shared/types/iterator.h"
#include "shared/util/log.h"

enum msginfo_states {
	mis_new = 1 << 0,
	mis_deleted = 1 << 1,
};

struct msginfo {
	msg_seq_t seq;
	uint32_t cooldown;
	uint32_t age;
	cx_bits_t send_to;
	enum msg_flags flags;
	uint16_t len;
	uint8_t state;
};

struct msg_queue {
	struct message msg;
	uint8_t *msgs, *cbuf;
	msg_seq_t seq;
	size_t len, cap, cbuf_len, cbuf_cap;
};

static void
del_msg(struct msginfo *mi)
{
	mi->state |= mis_deleted;
}

#define KiB 1024
#define MSG_BUF_INI_SIZE (256 * KiB)
#define MSG_BUF_MAX_SIZE (512 * KiB)

struct msg_queue *
msgq_init(void)
{
	struct msg_queue *mq = calloc(1, sizeof(struct msg_queue));

	mq->cap = MSG_BUF_INI_SIZE;
	mq->msgs = calloc(mq->cap, 1);

	return mq;
}

typedef enum iteration_result ((*msgq_for_each_cb)(void *ctx, struct msginfo *mi, uint8_t *msg));

static void
msgq_for_each(struct msg_queue *mq, msgq_for_each_cb cb, void *ctx)
{
	uint32_t i;
	for (i = 0; i < mq->len;) {
		struct msginfo *mi = (struct msginfo *)&mq->msgs[i];

		switch (cb(ctx, mi, &mq->msgs[i + sizeof(struct msginfo)])) {
		case ir_cont:
			break;
		case ir_done:
			return;
		}

		i += sizeof(struct msginfo) + mi->len;
	}
}

struct msgq_get_ctx {
	msg_seq_t seq;
	struct msginfo *mi;
	uint8_t *m;
};

static enum iteration_result
msgq_get_iter(void *_ctx, struct msginfo *mi, uint8_t *msg)
{
	struct msgq_get_ctx *ctx = _ctx;

	if (mi->seq == ctx->seq) {
		ctx->mi = mi;
		ctx->m = msg;
		return ir_done;
	} else {
		return ir_cont;
	}
}

static bool
msgq_get(struct msg_queue *q, msg_seq_t seq, struct msginfo **mi, uint8_t **m)
{
	struct msgq_get_ctx ctx = { .seq = seq };

	msgq_for_each(q, msgq_get_iter, &ctx);

	if (ctx.mi) {
		if (mi) {
			*mi = ctx.mi;
		}

		if (m) {
			*m = ctx.m;
		}

		return true;
	} else {
		return false;
	}
}

void
msgq_ack(struct msg_queue *q, msg_seq_t seq, cx_bits_t acker)
{
	/* TODO */
	return;

	struct msginfo *mh = NULL;

	if (!msgq_get(q, seq, &mh, NULL)) {
		return;
	} else if (mh->state & mis_deleted) {
		return;
	}

	mh->send_to &= ~acker;

	if (!mh->send_to) {
		del_msg(mh);
	}

	return;
}

void
msgq_add(struct msg_queue *q, struct message *msg, cx_bits_t send_to,
	enum msg_flags flags)
{
	if (!send_to) {
		return;
	}

	size_t elen = q->len + 2048; //sizeof(struct msginfo) + len;

	if (elen > MSG_BUF_MAX_SIZE) {
		L("queue is full, sorry");
	} else if (elen > q->cap) {
		q->cap = elen;

		q->msgs = realloc(q->msgs, q->cap);
		L("grew queue to %ld", q->cap);
		/* TODO: memeset new memory? */
	}

	struct msginfo *mi = (struct msginfo *)&q->msgs[q->len];
	q->len += sizeof(struct msginfo);

	*mi = (struct msginfo){
		.seq = ++q->seq,
		.send_to = send_to,
		.flags = flags,
		.state = mis_new,
		.len = pack_message(msg, q->msgs + q->len, q->cap - q->len),
	};

	q->len += mi->len;

	/* L("%3d | %ld...[%ld][%d]...%ld", */
	/* 	mi->seq, */
	/* 	q->len - sizeof(struct msginfo) - mi->len, */
	/* 	sizeof(struct msginfo), */
	/* 	mi->len, */
	/* 	q->len); */
}

struct msg_send_ctx {
	void *usr_ctx;
	msgq_send_all_iter sendf;
};

static enum iteration_result
msgq_send_iter(void *_ctx, struct msginfo *mi, uint8_t *msg)
{
	struct msg_send_ctx *ctx = _ctx;

	if (mi->state & mis_deleted) {
		return ir_cont;
	}

	if (mi->send_to && mi->cooldown == 0) {
		if (mi->state & mis_new) {
			mi->state &= ~mis_new;
		} else {
			/* TODO */
			//L("resending %x", i);
		}

		mi->cooldown = MSG_RESEND_AFTER;
		/* L("sending msg %d@%p", mi->len, (void *)msg); */
		ctx->sendf(ctx->usr_ctx, mi->send_to, mi->seq, mi->flags, msg, mi->len);

		if ((mi->flags & msgf_forget) || ++mi->age >= MSG_DESTROY_AFTER) {
			del_msg(mi);
		}
	} else if (mi->cooldown > 0) {
		--mi->cooldown;
	}

	return ir_cont;
}

void
msgq_send_all(struct msg_queue *q, void *usr_ctx, msgq_send_all_iter sendf)
{
	struct msg_send_ctx ctx = { .usr_ctx = usr_ctx, .sendf = sendf };

	msgq_for_each(q, msgq_send_iter, &ctx);
}

enum iteration_result
msgq_compact_iter(void *ctx, struct msginfo *mi, uint8_t *msg)
{
	struct msg_queue *q = ctx;

	/* if (mi->state & mis_deleted) { */
	return ir_cont;
	/* } */

	memcpy(&q->cbuf[q->cbuf_len], mi, sizeof(struct msginfo));
	q->cbuf_len += sizeof(struct msginfo);
	memcpy(&q->cbuf[q->cbuf_len], msg, mi->len);
	q->cbuf_len += mi->len;

	return ir_cont;
}

void
msgq_compact(struct msg_queue *q)
{
	if (q->cbuf_cap != q->cap) {
		q->cbuf_cap = q->cap;
		q->cbuf = realloc(q->cbuf, q->cap);
	}

	memset(q->cbuf, 0, q->cbuf_cap);

	q->cbuf_len = 0;

	msgq_for_each(q, msgq_compact_iter, q);

	uint8_t *tmp = q->msgs;
	q->msgs = q->cbuf;
	q->cbuf = tmp;
	q->len = q->cbuf_len;
}
