#include "posix.h"

#include <assert.h>
#include <string.h>

#include "shared/math/rand.h"
#include "shared/serialize/message.h"
#include "shared/types/sack.h"
#include "shared/util/log.h"

static size_t
pack_msg_wrapper(void *msg, uint8_t *buf, uint32_t blen)
{
	return pack_message(msg, buf, blen);
}

#define MAX_MSGS 32

struct msgs {
	struct sack msg_sk;
	struct {
		struct message msg;
		bool full;
	} msg_buf;
	struct message msgs[MAX_MSGS];
	uint32_t len;
};

static void
flush_msg_buf(struct msgs *msgs)
{
	if (!msgs->msg_buf.msg.mt) {
		return;
	}

	assert(msgs->len < MAX_MSGS);

	memcpy(&msgs->msgs[msgs->len], &msgs->msg_buf.msg, sizeof(struct message));
	++msgs->len;

	sack_stuff(&msgs->msg_sk, NULL, &msgs->msgs[msgs->len - 1]);

	memset(&msgs->msg_buf.msg, 0, sizeof(struct message));
}

static void
msg_add(struct msgs *msgs, enum message_type mt,
	void *msg)
{
	L("queueing  %s", inspect_message(mt, msg));

	bool appended = msgs->msg_buf.msg.count
			&& msgs->msg_buf.msg.mt == mt
			&& append_msg(&msgs->msg_buf.msg, msg);

	if (!appended) {
		flush_msg_buf(msgs);

		msgs->msg_buf.msg.mt = mt;

		if (!append_msg(&msgs->msg_buf.msg, msg)) {
			LOG_W("failed to append message");
			assert(false);
		}
	}
}

struct check_ctx {
	struct msgs *msgs;
	uint32_t msgi, smsgi;
};

static void
unpack_cb(void *_ctx, enum message_type mt, void *msg)
{
	struct check_ctx *ctx = _ctx;
	L("recvd   = %s", inspect_message(mt, msg));

	assert(mt == ctx->msgs->msgs[ctx->msgi].mt);
	assert(ctx->smsgi < ctx->msgs->msgs[ctx->msgi].count);

	void *orig = NULL;
	size_t len = 0;

	switch (mt) {
	case mt_poke:
		break;
	case mt_req:
		orig = &ctx->msgs->msgs[ctx->msgi].dat.req[ctx->smsgi];
		len = sizeof(struct msg_req);
		break;
	case mt_ent:
		orig = &ctx->msgs->msgs[ctx->msgi].dat.ent[ctx->smsgi];
		len = sizeof(struct msg_ent);
		break;
	case mt_tile:
		orig = &ctx->msgs->msgs[ctx->msgi].dat.tile[ctx->smsgi];
		len = sizeof(struct msg_tile);
		break;
	case mt_chunk:
		orig = &ctx->msgs->msgs[ctx->msgi].dat.chunk[ctx->smsgi];
		len = sizeof(struct msg_chunk);
		break;
	default:
		assert(false);
	}

	if (memcmp(msg, orig, len) != 0) {
		LOG_W("message does not match original");
		L("        = %s", inspect_message(mt, orig));
		assert(false);
	}

	++ctx->smsgi;
}


static enum del_iter_result
check_msg_cb(void *_ctx, void *_hdr, void *itm, uint16_t len)
{
	struct check_ctx *ctx = _ctx;

	ctx->smsgi = 0;

	unpack_message(itm, len, unpack_cb, ctx);

	++ctx->msgi;

	return dir_del;
}

static void
check_msgs(struct msgs *msgs)
{
	flush_msg_buf(msgs);
	struct check_ctx ctx = { .msgs = msgs, };

	/* L("sanity check"); */
	/* uint32_t i; */
	/* for (i = 0; i < msgs->msgs[0].count; ++i) { */
	/* 	unpack_cb(&ctx, msgs->msgs[0].mt, &msgs->msgs[0].dat.req[i]); */
	/* } */

	/* L("real check"); */
	sack_iter(&msgs->msg_sk, &ctx, check_msg_cb);

	memset(msgs->msgs, 0, sizeof(struct message) * MAX_MSGS);
	msgs->len = 0;

	assert(msgs->msg_sk.len == 0);
}

void
request_missing_chunks(struct msgs *msgs, struct rectangle *r)
{
	struct point onp, np = onp = nearest_chunk(&r->pos);

	for (; np.x < r->pos.x + r->width; np.x += CHUNK_SIZE) {
		for (np.y = onp.y; np.y < r->pos.y + r->height; np.y += CHUNK_SIZE) {
			if (np.x < 0 || np.y < 0) {
				continue;
			}

			struct msg_req msg = {
				.mt = rmt_chunk,
				.dat.chunk = np
			};

			msg_add(msgs, mt_req, &msg);
		}
	}
}

int
main(void)
{
	log_init();
	log_level = ll_debug;

	struct msgs msgs = { 0 };
	sack_init(&msgs.msg_sk, 0, 1024 * 1024, pack_msg_wrapper);

	struct rectangle r = {
		.pos = { 0, 0 },
		.width = 32,
		.height = 32,
	};


	request_missing_chunks(&msgs, &r);
	check_msgs(&msgs);

	r.pos.y = 32;

	request_missing_chunks(&msgs, &r);
	check_msgs(&msgs);

/* 	uint32_t i; */
/* 	for (i = 0; i < 64; ++i) { */
/* 		if (drand48() > 0.5) { */
/* 			r.pos.x += (rand_uniform(8) - 4) * 16; */
/* 			r.pos.y += (rand_uniform(8) - 4) * 16; */
/* 		} */

/* 		L("%d, %d", r.pos.x, r.pos.y); */

/* 		request_missing_chunks(&msgs, &r); */
/* 		check_msgs(&msgs); */
/* 	} */
}
