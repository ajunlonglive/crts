#include "posix.h"

#include <assert.h>
#include <string.h>

#include "shared/serialize/to_disk.h"
#include "shared/serialize/world.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

#define BLEN (724 * 500)

void
write_chunks(FILE *f, struct chunks *chunks)
{
	uint32_t i, len = hdarr_len(chunks->hd), packed;
	uint8_t buf[BLEN] = { 0 };

	packed = 0;
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
	L("wrote %d/%d chunks", i, len);
	fflush(f);
}

void
read_chunks(FILE *f, struct chunks *chunks)
{
	size_t b, unpacked = 0, rem = 0;
	uint8_t buf[BLEN];
	struct chunk c;

	while ((b = fread(&buf[rem], 1, BLEN - rem, f))) {
		unpacked = 0;
		do {
			uint32_t a = unpack_chunk(&c, &buf[unpacked], BLEN);
			unpacked += a;

			hdarr_set(chunks->hd, &c.pos, &c);

		} while (unpacked < (BLEN / 2));

		rem = BLEN - unpacked;

		memmove(buf, &buf[unpacked], rem);
	}
}
