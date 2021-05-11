#include "posix.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared/serialize/coder.h"
#include "shared/util/log.h"

#define LIM 27
#define BLEN 512

static bool
test_string(const char *str)
{
	uint8_t buf[BLEN] = { 0 };
	uint32_t i;
	size_t len = strlen(str);

	struct ac_coder packer = { 0 };

	ac_pack_init(&packer, buf, BLEN);

	packer.lim = LIM;

	for (i = 0; i < len; ++i) {
		assert(str[i] - 'a' < LIM);
		ac_pack(&packer, str[i] - 'a');
	}

	ac_pack_finish(&packer);

	uint32_t vbuf[BLEN] = { 0 };
	struct ac_decoder unpacker;

	ac_unpack_init(&unpacker, buf, ac_coder_len(&packer));
	unpacker.lim = LIM;
	ac_unpack(&unpacker, vbuf, len);

	for (i = 0; i < len; ++i) {
		if ((uint8_t)str[i] != vbuf[i] + 'a') {
			return false;
		}
	}

	return true;
}

static bool
test_uint16_t(uint16_t i)
{
	uint8_t buf[16] = { 0 };
	struct ac_coder cod = { 0 };

	ac_pack_init(&cod, buf, 16);
	cod.lim = UINT16_MAX;
	ac_pack(&cod, i);
	ac_pack_finish(&cod);

	struct ac_decoder dec = { 0 };

	ac_unpack_init(&dec, buf, ac_coder_len(&cod));
	uint32_t v;
	dec.lim = UINT16_MAX;
	ac_unpack(&dec, &v, 1);

	assert(ac_coder_len(&cod) == ac_decoder_len(&dec));
	return v == i;
}

static void
pack_first_part(struct ac_coder *c)
{
	c->lim = UINT16_MAX;
	ac_pack(c, 1);
}

static void
pack_second_part(struct ac_coder *c)
{
	c->lim = 4;
	ac_pack(c, 2);
}

#define ALEN 512
#define TPMBUF (16 * ALEN)
static void
test_packing_methods(void)
{
	uint32_t i;
	struct ac_coder c1 = { 0 }, c2 = { 0 };
	uint8_t buf1[TPMBUF] = { 0 }, buf2[TPMBUF] = { 0 };
	ac_pack_init(&c1, buf1, TPMBUF);
	ac_pack_init(&c2, buf2, TPMBUF);

	/* method 1: array of structs */
	for (i = 0; i < ALEN; ++i) {
		pack_first_part(&c1);
		pack_second_part(&c1);
	}

	ac_pack_finish(&c1);

	/* method 2: struct of arrays */
	for (i = 0; i < ALEN; ++i) {
		pack_first_part(&c2);
	}

	for (i = 0; i < ALEN; ++i) {
		pack_second_part(&c2);
	}

	ac_pack_finish(&c2);

	L(log_misc, "AOS: %ld (%d) | SOA: %ld (%d) | NONE: %d",
		ac_coder_len(&c1), c1.bufi,
		ac_coder_len(&c2), c2.bufi,
		ALEN * 8);
}

int32_t
main(int32_t argc, const char *const argv[])
{
	log_init();
	log_level = ll_debug;

	for (uint32_t i = 0; i < 1000; ++i) {
		char str[256] = { 0 };
		for (uint32_t j = 0; j < 255; ++j) {
			str[j] = (rand() % 26) + 'a';
		}

		if (!test_string(str)) {
			return 1;
		}
	}

	for (uint32_t i = 0; i < UINT16_MAX; ++i) {
		bool res = test_uint16_t(i);
		if (!res) {
			L(log_misc, "test failed: %d", i);
		}

		assert(res);
	}

	test_packing_methods();

	return 0;
}
