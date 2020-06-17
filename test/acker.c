#include "posix.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "shared/net/ack.h"
#include "shared/serialize/net.h"
#include "shared/types/iterator.h"
#include "shared/util/log.h"

#define MSGS 321

msg_seq_t to_ack[MSGS] = { 0 };
msg_seq_t ackd[MSGS] = { 0 };
msg_seq_t really_ackd[MSGS] = { 0 };

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
	size_t block = FRAME_LEN;

	for (i = 0; i < ACK_BLOCKS; ++i) {
		printf("block %2ld: %4ld | %08x\n", i, block, a->acks[i]);
		block -= ACK_BLOCK_LEN;
	}
}

int
main(int argc, char **argv)
{
	struct acks a, b;
	char buf[2048];
	msg_seq_t id;
	size_t i;

	ack_clear_all(&a);

	id = rand();

	for (i = 0; i < MSGS; ++i) {
		to_ack[i] = (id + i) % MSG_ID_LIM;
		really_ackd[i] = 1;
	}

	for (i = 0; i < MSGS; ++i) {
		ack_set(&a, to_ack[i]);
	}

	pack_acks(&a, buf);
	unpack_acks(&b, buf);

	ack_iter(&b, NULL, really_acked);

	for (i = 0; i < MSGS; ++i) {
		assert(ack_check(&b, to_ack[i]));
	}

	for (i = 0; i < MSGS; ++i) {
		assert(ackd[i] == really_ackd[i]);
	}

	return 0;
}
