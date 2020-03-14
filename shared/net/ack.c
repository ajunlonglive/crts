#include <assert.h>
#include <string.h>

#include "shared/net/ack.h"
#include "shared/net/defs.h"
#include "shared/types/iterator.h"
#include "shared/util/log.h"

static void
shift_blocks(struct acks *a, size_t amnt)
{
	assert(false);
	assert(amnt < ACK_BLOCKS);

	L("shifting %ld", amnt);

	/*
	 * | a | b | c | d |
	 *    \   \   \   \
	 *     \   \   \   \
	 *      \   \   \   \
	 * | 0 | a | b | c |
	 */
	memmove(a->acks + amnt, a->acks, sizeof(msg_ack_t) * (ACK_BLOCKS - amnt));
	memset(a->acks, 0, sizeof(msg_ack_t) * amnt);
}

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
	a->initialized = true;
	a->leader = FRAME_LEN;
}

void
ack_set(struct acks *a, msg_seq_t new)
{
	assert(a->initialized);
	msg_seq_t block;
	size_t bi;

	block = calculate_block(new);
	bi = calculate_block_index(block);

	if (bi >= ACK_BLOCKS) {
		return;
	} else if (block > a->leader) {
		shift_blocks(a, (block - a->leader) / ACK_BLOCK_LEN);
		a->leader = block;
	}

	a->acks[bi] |= 1 << (new & (ACK_BLOCK_LEN - 1));
}

bool
ack_check(struct acks *a, msg_seq_t id)
{
	assert(a->initialized);
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

	assert(a->initialized);

	for (i = 0; i < ACK_BLOCKS; ++i) {
		block = (a->leader - (i * ACK_BLOCK_LEN));

		for (j = 0; j < ACK_BLOCK_LEN; ++j) {
			if (a->acks[i] & (1 << j)) {
				func(ctx, block - (ACK_BLOCK_LEN - j));
			}
		}
	}
}
