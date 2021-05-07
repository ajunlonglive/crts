#include "posix.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "shared/serialize/chunk.h"
#include "shared/serialize/to_disk.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

#define BLEN 1048576lu

void
write_chunks(FILE *f, struct chunks *chunks)
{
	uint32_t i, len = hdarr_len(&chunks->hd), packed = 0;
	uint8_t buf[BLEN] = { 0 };

	for (i = 0; i < len; ++i) {
		struct chunk *c = darr_get(&chunks->hd.darr, i);
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
	const uint64_t est_size = 1000; // TODO: this doesn't seem very robust...

	uint64_t b, unpacked = 0, rem = 0, count = 0;
	uint8_t *buf = z_calloc(BLEN, 1);
	struct chunk c = { 0 };

	while ((b = fread(&buf[rem], 1, BLEN - rem, f))) {
		unpacked = 0;
		b += rem;

		assert(b > est_size);
		do {
			uint32_t a = unpack_chunk(&c, &buf[unpacked], b);
			unpacked += a;

			hdarr_set(&chunks->hd, &c.pos, &c);

			++count;
		} while (unpacked < (b - est_size));

		rem = BLEN - unpacked;

		memmove(buf, &buf[unpacked], rem);
	}

	L("read %ld chunks", count);
}

bool
load_world_from_path(const char *path, struct chunks *chunks)
{
	FILE *f;

	if (strcmp(path, "-") == 0) {
		LOG_I("loading world from stdin");
		f = stdin;
	} else if ((f = fopen(path, "r"))) {
		LOG_I("loading world from %s", path);
	} else {
		LOG_W("unable to read '%s': %s\n", path, strerror(errno));
		return false;
	}

	read_chunks(f, chunks);

	fclose(f);

	return true;
}
