#include "posix.h"

#include <string.h>

#include "shared/msgr/transport/rudp/seq_buf.h"
#include "shared/msgr/transport/rudp/util.h"
#include "shared/util/log.h"

#define SEQ_BUF_SIZE 1024
#define SEQ_BUF_MOD 0x3ff

enum seq_buf_flags {
	seq_buf_flag_set = 1 << 0,
};

struct seq_buf_entry {
	uint16_t seq;
	uint16_t flags;
	char data[];
};

void
seq_buf_init(struct seq_buf *sb, uint32_t isize)
{
	darr_init(&sb->dat, isize + sizeof(struct seq_buf_entry));
	darr_grow_to(&sb->dat, SEQ_BUF_SIZE);
	memset(sb->dat.e, 0, sb->dat.item_size * sb->dat.cap);
}

void
seq_buf_destroy(struct seq_buf *sb)
{
	darr_destroy(&sb->dat);
}

void *
seq_buf_get(struct seq_buf *sb, uint16_t seq)
{
	struct seq_buf_entry *sbe = darr_get(&sb->dat, seq & SEQ_BUF_MOD);

	/* L("getting %p %d, %s", (void *)sb, seq, (sbe->flags & seq_buf_flag_set) && sbe->seq == seq ? "yes" : "no"); */
	if ((sbe->flags & seq_buf_flag_set) && sbe->seq == seq) {
		return sbe->data;
	} else {
		return NULL;
	}
}

void
seq_buf_clear_range(struct seq_buf *sb, uint16_t start, uint16_t end)
{
	// TODO
}

void *
seq_buf_insert(struct seq_buf *sb, uint16_t seq)
{
	if (seq_lt(seq, sb->head - SEQ_BUF_SIZE)) {
		return NULL;
	} else if (seq_gt(seq, sb->head)) {
		seq_buf_clear_range(sb, sb->head, seq);
		sb->head = seq;
	}

	struct seq_buf_entry *sbe = darr_get(&sb->dat, seq & SEQ_BUF_MOD);
	sbe->seq = seq;
	sbe->flags |= seq_buf_flag_set;
	/* L("inserting %p, %d", (void *)sb, seq); */

	return sbe->data;
}

uint32_t
seq_buf_gen_ack_bits(struct seq_buf *sb)
{
	uint32_t ack_bits = 0;
	uint16_t i;

	for (i = 0; i < 32; ++i) {
		if (seq_buf_get(sb, sb->head - i)) {
			ack_bits |= 1 << i;
		}
	}

	return ack_bits;
}
