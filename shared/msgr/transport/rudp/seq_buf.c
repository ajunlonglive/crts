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
	if (seq_lt(seq, sb->head - (uint16_t)SEQ_BUF_SIZE)) {
		assert(false);
		return NULL;
	} else if (seq_gt(seq, sb->head)) {
		seq_buf_clear_range(sb, sb->head, seq);
		sb->head = seq;
	}

	struct seq_buf_entry *sbe = darr_get(&sb->dat, seq & SEQ_BUF_MOD);
	sbe->seq = seq;
	sbe->flags |= seq_buf_flag_set;

	return sbe->data;
}

void
seq_buf_gen_ack_bits(struct seq_buf *sb, uint32_t *buf, uint32_t blen,
	uint32_t start)
{
	uint16_t i = 0, bufi = 0;
	uint8_t biti = 0;

	while (bufi < blen) {
		if (seq_buf_get(sb, start - i)) {
			buf[bufi] |= 1 << biti;
		}

		i++;
		++biti;

		if (!(i & 31)) {
			++bufi;
			biti = 0;
		}
	}
}

uint32_t
seq_buf_gen_ack_bits_from_start(struct seq_buf *sb)
{
	uint32_t ack_bits = 0;

	seq_buf_gen_ack_bits(sb, &ack_bits, 1, sb->head);

	return ack_bits;
}
