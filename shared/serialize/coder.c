#include "posix.h"

#include <assert.h>
#include <stddef.h>

#include "shared/serialize/coder.h"

void
ac_init(struct ac_coder *c)
{
	c->ceil = ~0;
	c->floor = 0;
}

void
write_bit(struct ac_coder *c, uint8_t bit)
{
#define WB() c->buf[c->bufi / 8] |= 1 << (c->bufi % 8)
	if (c->pending) {
		if (bit) {
			WB();
			c->bufi += c->pending + 1;
		} else {
			++c->bufi;

			while (c->pending--) {
				WB();
				++c->bufi;
			}
		}

		c->pending = 0;
	} else {
		if (bit) {
			WB();
		}

		++c->bufi;
	}
#undef WB
}

#define msb  (1u << 31)
#define smsb (1u << 30)

void
ac_pack(struct ac_coder *c, uint32_t val)
{
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
ac_unpack(const struct ac_coder *c, uint32_t buf[], size_t len)
{
	uint32_t ceil = ~0u, floor = 0u, val = 0, b = 0, bi = 0, i;

	for (b = 0; b < 32; ++b) {
		val <<= 1;

		if (b < c->bufi && c->buf[b / 8] & (1 << (b % 8))) {
			val |= 1;
		}
	}

	for (i = 0; i < len; ++i) {
		uint32_t range = (ceil - (floor)) / c->lim;

		uint32_t sval = (val - floor) / range;

		buf[bi++] = sval;

		ceil = floor + range * (sval + 1);
		floor = floor + range * sval;

		assert(ceil > floor);

		for (;;) {
			if (ceil < msb) {
				/* do nothing */
			} else if (floor >= msb) {
				val -= msb;
				floor -= msb;
				ceil -= msb;
			} else if (floor >= smsb && ceil < (msb | smsb)) {
				val -= smsb;
				floor -= smsb;
				ceil -= smsb;
			} else {
				break;
			}

			floor <<= 1;

			ceil <<= 1;
			ceil |= 1;

			val <<= 1;
			if (b < c->bufi && c->buf[b / 8] & (1 << (b % 8))) {
				val |= 1;
			}
			++b;
		}
	}
}
