#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "shared/net/msg_queue.h"
#include "shared/types/darr.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"

#define MSG_ID_LIM 512

struct msg_queue {
	struct hdarr *msgs;
	struct darr *to_flush;
	size_t msgsize;
	msg_seq_t seq;
	msg_seq_t ids;
};

struct msginfo {
	msg_seq_t id;
	msg_seq_t seq;
	cx_bits_t send_to;
	bool sent;
};

static void *
msgq_key_getter(void *_mi)
{
	struct msginfo *mi = _mi;

	return &mi->id;
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
	struct msginfo hdr = { .id = (q->ids = (q->ids + 1) % MSG_ID_LIM),
			       .sent = false, .send_to = send_to };
	hdr.send_to = 0xff;

	if (hdarr_get(q->msgs, &hdr.id) != NULL) {
		L("overwriting msg %d", hdr.id);
	}

	size_t i = hdarr_set(q->msgs, &hdr.id, &hdr);

	//L("adding message, %d, %d, %d", hdr.id, hdr.seq, hdr.send_to);

	return darr_point_at(hdarr_darr(q->msgs), i) + sizeof(struct msginfo);
}

void
msgq_ack(struct msg_queue *q, msg_seq_t seq, msg_ack_t acks, cx_bits_t acker)
{
	/*
	   size_t i;

	   struct msginfo *mh;
	   msg_ack_t cack;
	   for (i = 0; i < FRAMELEN; ++i) {

	                cack = 1 << i;
	           if ((acks & cack) && (mh = msgq_geth(q, seq + i, &ix)) != NULL) {
	                mh->send_to &= ~acker;

	                if (!mh->send_to) {
	                        darr_push(q->to_flush, &ix);
	                }
	           }
	   }
	 */
}

void
msgq_send_all(struct msg_queue *q, void *ctx, msgq_send_all_iter send)
{
	size_t i, len = hdarr_len(q->msgs);
	char *mem;
	struct msginfo *mh;

	for (i = 0; i < len; ++i) {
		mem = darr_point_at(hdarr_darr(q->msgs), i);
		mh = (struct msginfo *)mem;

		if (!mh->sent) { // || mh->send_to) {
			mem += sizeof(struct msginfo);

			mh->seq = ++q->seq;
			send(ctx, mh->send_to, mh->seq, mem);
			mh->sent = true;
			darr_push(q->to_flush, &mh->id);
		}
	}
}

static enum iteration_result
flush_msg(void *_q, void *_id)
{
	msg_seq_t *id = _id;
	struct msg_queue *q = _q;

	//L("flushing %u", *id);

	hdarr_del_p(q->msgs, id, true);
	return ir_cont;
}

void
msgq_flush(struct msg_queue *q)
{
	//L("flushing %ld msgs", darr_len(q->to_flush));
	darr_for_each(q->to_flush, q, flush_msg);
	//L("queued msgs: %ld", hdarr_len(q->msgs));
	darr_clear(q->to_flush);
}
