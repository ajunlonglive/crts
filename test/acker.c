#include "posix.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "shared/net/msg_queue.h"
#include "shared/serialize/message.h"
#include "shared/serialize/net.h"
#include "shared/types/iterator.h"
#include "shared/util/log.h"

void
inspect_full_msg(struct message *msg)
{
	uint32_t i;
	void *smsg;

	L("msg %d[%d] {", msg->mt, msg->count);

	for (i = 0; i < msg->count; ++i) {
		switch (msg->mt) {
		case mt_poke:
			continue;
		case mt_req:
			smsg = &msg->dat.req[i];
			break;
		case mt_ent:
			smsg = &msg->dat.ent[i];
			break;
		case mt_action:
			smsg = &msg->dat.action[i];
			break;
		case mt_tile:
			smsg = &msg->dat.tile[i];
			break;
		case mt_chunk:
			smsg = &msg->dat.chunk[i];
			break;
		default:
			assert(false);
			return;
		}

		L("  %s,", inspect_message(msg->mt, smsg));
	}
	L("}");
}

void
unpack_msg_cb(void *_ctx, enum message_type mt, void *msg)
{
	struct message *om = _ctx;
	assert(mt == om->mt);
	L("got: %s", inspect_message(om->mt, msg));
}

static void
sendf(void *_ctx, cx_bits_t sendto, msg_seq_t seq, enum msg_flags f, void *msg,
	uint16_t len)
{
	printf("unpacking msg %d %d %d %d\n", sendto, seq, f, len);

	unpack_message(msg, len, unpack_msg_cb, _ctx);
}

int32_t
main(int32_t argc, const char *const argv[])
{
	log_init();
	log_level = ll_debug;

	struct msg_queue mqa = { 0 };
	msgq_init(&mqa);
	mqa.seq = 23;
	mqa.len = 24;

	struct message msg = {
		.mt =  mt_req,
		.count = 8,
		.dat.req = {
			{ .mt = rmt_chunk, .dat = { 16, 16 } },
			{ .mt = rmt_chunk, .dat = { 16, 32 } },
			{ .mt = rmt_chunk, .dat = { 32, 16 } },
			{ .mt = rmt_chunk, .dat = { 32, 32 } },
			{ .mt = rmt_chunk, .dat = { 48, 16 } },
			{ .mt = rmt_chunk, .dat = { 48, 32 } },
			{ .mt = rmt_chunk, .dat = { 64, 16 } },
			{ .mt = rmt_chunk, .dat = { 64, 32 } },
		}
	};

	msgq_add(&mqa, &msg, 1, 1);
	msgq_send_all(&mqa, &msg, sendf);

	L("compacting");
	msgq_compact(&mqa);

	msgq_add(&mqa, &msg, 1, 0);
	msgq_add(&mqa, &msg, 1, 0);
	msgq_add(&mqa, &msg, 1, 0);
	msgq_send_all(&mqa, &msg, sendf);

	uint32_t i;
	for (i = 0; i < UINT16_MAX; ++i) {
		struct msg_hdr hdr = { .seq = i, .kind = mk_ack };

		uint8_t buf[256] = { 0 };
		size_t plen = pack_msg_hdr(&hdr, buf, 256);
		struct msg_hdr uhdr = { 0 };
		size_t ulen = unpack_msg_hdr(&uhdr, buf, 256);

		if (plen != ulen) {
			L("buffer size mismatch %ld / %ld", plen, ulen);
		}

		assert(plen == ulen);
		assert(hdr.seq == uhdr.seq);
		assert(hdr.kind == uhdr.kind);
	}
}
