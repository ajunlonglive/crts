#include "posix.h"

#include <assert.h>
#include <string.h>

#include "shared/net/ack.h"
#include "shared/net/defs.h"
#include "shared/types/iterator.h"
#include "shared/util/log.h"

static msg_seq_t
calculate_block(msg_seq_t id)
{
	return id + ACK_BLOCK_LEN - (id & (ACK_BLOCK_LEN - 1));
}

static size_t
calculate_block_index(msg_seq_t block)
{
	return ACK_BLOCKS - (block / ACK_BLOCK_LEN);
}

void
ack_clear_all(struct acks *a)
{
	memset(a, 0, sizeof(struct acks));
}

void
ack_set(struct acks *a, msg_seq_t new)
{
	msg_seq_t block;
	size_t bi;

	block = calculate_block(new);
	bi = calculate_block_index(block);

	assert(block <= FRAME_LEN);
	assert(bi < ACK_BLOCKS);

	a->acks[bi] |= 1 << (new & (ACK_BLOCK_LEN - 1));
}

bool
ack_check(struct acks *a, msg_seq_t id)
{
	size_t bi;

	if ((bi = calculate_block_index(calculate_block(id))) > ACK_BLOCKS) {
		return false;
	} else {
		return a->acks[bi] & (1 << (id & (ACK_BLOCK_LEN - 1)));
	}
}

void
ack_iter(struct acks *a, void *ctx, ack_iter_func func)
{
	size_t i, j;
	msg_seq_t block;

	for (i = 0; i < ACK_BLOCKS; ++i) {
		block = (FRAME_LEN - (i * ACK_BLOCK_LEN));

		for (j = 0; j < ACK_BLOCK_LEN; ++j) {
			if (a->acks[i] & (1 << j)) {
				func(ctx, block - (ACK_BLOCK_LEN - j));
			}
		}
	}
}
