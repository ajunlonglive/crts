#include "posix.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared/serialize/coder.h"

#define LIM 27

static bool
test_string(const char *str)
{
	uint8_t buf[256] = { 0 };
	uint32_t i;
	size_t len = strlen(str);

	struct ac_coder packer = { LIM, .buf = buf, .buflen = 256 };

	ac_pack_init(&packer);

	for (i = 0; i < len; ++i) {
		assert(str[i] - 'a' < LIM);
		ac_pack(&packer, str[i] - 'a');
	}

	ac_pack_finish(&packer);

	uint32_t vbuf[256] = { 0 };
	struct ac_decoder unpacker = { LIM, .buf = buf, .buflen = packer.bufi };

	ac_unpack_init(&unpacker);
	ac_unpack(&unpacker, vbuf, len);

	for (i = 0; i < len; ++i) {
		if ((uint8_t)str[i] != vbuf[i] + 'a') {
			return false;
		}
	}

	return true;
}

int32_t
main(int32_t argc, const char *const argv[])
{
	for (uint32_t i = 0; i < 1000; ++i) {
		char str[256] = { 0 };
		for (uint32_t j = 0; j < 255; ++j) {
			str[j] = (rand() % 26) + 'a';
		}

		if (!test_string(str)) {
			return 1;
		}
	}
	return 0;
}
