#include "posix.h"

#include <assert.h>
#include <string.h>

#include "shared/serialize/chunk.h"
#include "shared/serialize/to_disk.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

#define BLEN 0xffff

void
write_chunks(FILE *f, struct chunks *chunks)
{
	uint32_t i, len = hdarr_len(chunks->hd), packed = 0;
	uint8_t buf[BLEN] = { 0 };

	for (i = 0; i < len; ++i) {
		struct chunk *c = darr_get(hdarr_darr(chunks->hd), i);
		packed += pack_chunk(c, &buf[packed], BLEN - packed);

		if (BLEN - packed < 1000) {
			fwrite(buf, 1, packed, f);
			packed = 0;
			memset(buf, 0, BLEN);
		}
	}

	fwrite(buf, 1, packed, f);
	L("wrote %d chunks", len);
}

void
read_chunks(FILE *f, struct chunks *chunks)
{
	size_t b, unpacked = 0, rem = 0, count = 0;
	uint8_t buf[BLEN] = { 0 };
	struct chunk c = { 0 };

	while ((b = fread(&buf[rem], 1, BLEN - rem, f))) {
		unpacked = 0;
		b += rem;
		do {
			uint32_t a = unpack_chunk(&c, &buf[unpacked], b);
			unpacked += a;

			hdarr_set(chunks->hd, &c.pos, &c);

			++count;
		} while (unpacked < (b - 1000));

		rem = BLEN - unpacked;

		memmove(buf, &buf[unpacked], rem);
	}

	L("read %ld chunks", count);
}
