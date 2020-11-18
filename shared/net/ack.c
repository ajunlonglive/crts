#include "posix.h"

#include <assert.h>
#include <string.h>

#include "shared/net/ack.h"
#include "shared/net/defs.h"
#include "shared/net/msg_queue.h"
#include "shared/types/iterator.h"
#include "shared/util/log.h"

typedef uint32_t ack_t;

#define ACK_CAP (sizeof(ack_t) * 8)


void
ack_init(struct hash *hash)
{
	hash_init(hash, 2048, sizeof(msg_seq_t));
}

void
ack_clear_all(struct hash *ags)
{
	hash_clear(ags);
}

void
ack_set(struct hash *ags, msg_seq_t new)
{
	const size_t *val;
	size_t nv;

	msg_seq_t key = new % ACK_CAP;

	if ((val = hash_get(ags, &key))) {
		nv = *val | 1 << (new - key);
	} else {
		nv = 1 << (new - key);
	}

	hash_set(ags, &key, nv);
}

bool
ack_check(struct hash *ags, msg_seq_t new)
{
	const size_t *val;
	msg_seq_t key = new % ACK_CAP;

	return (val = hash_get(ags, &key))
	       && *val & (1 << (new - key));
}

struct ack_msgq_ctx {
	struct msg_queue *q;
	cx_bits_t acker;
};

enum iteration_result
ack_msgq_iter(void *_ctx, void *_key, size_t ack)
{
	uint32_t i;
	struct ack_msgq_ctx *ctx = _ctx;
	msg_seq_t seq = *(msg_seq_t*)_key;

	for (i = 0; i < ACK_CAP; ++i) {
		if (i & ack) {
			msgq_ack(ctx->q, seq + i, ctx->acker);
		}
	}
	return ir_cont;
}

void
ack_msgq(struct hash *ags, struct msg_queue *q, cx_bits_t acker)
{
	struct ack_msgq_ctx ctx = { q, acker };
	hash_for_each_with_keys(ags, &ctx, ack_msgq_iter);
}
