#include "posix.h"

#include <assert.h>
#include <stddef.h>

#include "shared/serialize/coder.h"

void
ac_pack_init(struct ac_coder *c)
{
	c->ceil = ~0;
	/* c->floor = 0; */
}

static void
write_one_bit(struct ac_coder *c)
{
	assert(c->bufi / 8 < c->buflen);
	c->buf[c->bufi / 8] |= 1 << (c->bufi % 8);
}

void
write_bit(struct ac_coder *c, uint8_t bit)
{
	if (c->pending) {
		if (bit) {
			write_one_bit(c);
			c->bufi += c->pending + 1;
		} else {
			++c->bufi;

			while (c->pending--) {
				write_one_bit(c);
				++c->bufi;
			}
		}

		c->pending = 0;
	} else {
		if (bit) {
			write_one_bit(c);
		}

		++c->bufi;
	}
}

#define msb  (1u << 31)
#define smsb (1u << 30)

void
ac_pack(struct ac_coder *c, uint32_t val)
{
	assert(c->lim && c->ceil && c->buflen);

	uint32_t range = (c->ceil - (c->floor)) / c->lim;

	assert(c->ceil > c->floor);

	c->ceil = c->floor + range * (val + 1);
	c->floor = c->floor + range * val;

	for (;;) {
		if (c->ceil < msb) {
			write_bit(c, 0);
		} else if (c->floor >= msb) {
			write_bit(c, 1);
		} else if (c->floor >= smsb && c->ceil < (msb | smsb)) {
			++c->pending;

			c->floor <<= 1;
			c->floor &= ~msb;

			c->ceil <<= 1;
			c->ceil |= (msb | 1);
			continue;
		} else {
			break;
		}

		c->floor <<= 1;
		c->ceil <<= 1;
		c->ceil |= 1;
	}
}

void
ac_pack_finish(struct ac_coder *c)
{
	++c->pending;

	if (c->floor < smsb) {
		write_bit(c, 0);
	} else {
		write_bit(c, 1);
	}
}

void
ac_unpack_init(struct ac_decoder *c)
{
	c->ceil = ~0;

	for (c->bufi = 0; c->bufi < c->buflen && c->bufi < 32; ++c->bufi) {
		c->val <<= 1;

		if (c->buf[c->bufi / 8] & (1 << (c->bufi % 8))) {
			c->val |= 1;
		}
	}
}

void
ac_unpack(struct ac_decoder *c, uint32_t out[], size_t len)
{
	assert(c->lim && c->ceil && c->buflen);

	uint32_t i;

	for (i = 0; i < len; ++i) {
		uint32_t range = (c->ceil - (c->floor)) / c->lim;

		uint32_t sval = (c->val - c->floor) / range;

		out[i] = sval;

		c->ceil = c->floor + range * (sval + 1);
		c->floor = c->floor + range * sval;

		assert(c->ceil > c->floor);

		for (;;) {
			if (c->ceil < msb) {
				/* do nothing */
			} else if (c->floor >= msb) {
				c->val -= msb;
				c->floor -= msb;
				c->ceil -= msb;
			} else if (c->floor >= smsb && c->ceil < (msb | smsb)) {
				c->val -= smsb;
				c->floor -= smsb;
				c->ceil -= smsb;
			} else {
				break;
			}

			c->floor <<= 1;

			c->ceil <<= 1;
			c->ceil |= 1;

			c->val <<= 1;

			if (c->bufi < c->buflen) {
				if (c->buf[c->bufi / 8] & (1 << (c->bufi % 8))) {
					c->val |= 1;
				}
				++c->bufi;
			}
		}
	}
}