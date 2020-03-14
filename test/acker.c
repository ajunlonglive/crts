#define _XOPEN_SOURCE 500

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "shared/net/ack.h"
#include "shared/types/iterator.h"
#include "shared/util/log.h"

#define MSGS 512

msg_seq_t to_ack[MSGS];
msg_seq_t ackd[MSGS];

static enum iteration_result
really_acked(void *_, msg_seq_t seq)
{
	size_t i;
	bool found = false;
	for (i = 0; i < MSGS; ++i) {
		if (to_ack[i] == seq) {
			++ackd[i];
			found = true;
			break;
		}
	}

	assert(found);

	return ir_cont;
}

void
print_acks(struct acks *a)
{
	size_t i;
	size_t block = a->leader;

	for (i = 0; i < ACK_BLOCKS; ++i) {
		printf("block %2ld: %4ld | %08x\n", i, block, a->acks[i]);
		block -= ACK_BLOCK_LEN;
	}
}

int
main(int argc, char **argv)
{
	struct acks a;
	size_t i;

	ack_clear_all(&a);

	for (i = 0; i < MSGS; ++i) {
		to_ack[i] = i % MSG_ID_LIM;
		ackd[i] = 0;
	}

	for (i = 0; i < MSGS; ++i) {
		ack_set(&a, to_ack[i]);
	}

	ack_iter(&a, NULL, really_acked);

	for (i = 0; i < MSGS; ++i) {
		assert(ackd[i] == 1);
	}

	for (i = 0; i < MSGS; ++i) {
		assert(ack_check(&a, to_ack[i]));
	}

	return 0;
}
