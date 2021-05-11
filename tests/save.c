#include "posix.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "shared/serialize/chunk.h"
#include "shared/serialize/coder.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

void
print_chunk(struct chunk *c)
{
	uint32_t x, y;

	printf("chunk @ (%3d, %3d)\n", c->pos.x, c->pos.y);

	for (y = 0; y < CHUNK_SIZE; ++y) {
		for (x = 0; x < CHUNK_SIZE; ++x) {
			printf("\033[3%dm%c\033[0m ",
				(uint32_t)(c->heights[x][y] / 4 + 0.5f) + 1,
				c->tiles[x][y] + ' ');
		}
		printf("\n");
	}
}

static void
randomize_chunk(struct chunk *c)
{
	struct point r = { rand() % 2048,
			   rand() % 2048 };

	c->pos = nearest_chunk(&r);

	struct pointf p = { rand() % CHUNK_SIZE, rand() % CHUNK_SIZE };

	uint32_t x, y;
	for (y = 0; y < CHUNK_SIZE; ++y) {
		for (x = 0; x < CHUNK_SIZE; ++x) {
			struct pointf q = { x, y };

			float d = sqrtf(fsqdist(&p, &q));

			c->tiles[x][y] = (uint32_t)d % tile_count;
			c->heights[x][y] = roundf(d * 100.0f) / 100.0f;
		}
	}
}

static bool
chunks_eql(const struct chunk *a, const struct chunk *b, float err)
{
	if (!points_equal(&a->pos, &b->pos)) {
		printf("points not equal (%d, %d) and (%d, %d)\n",
			a->pos.x, a->pos.y, b->pos.x, b->pos.y);
		return false;
	}

	uint32_t x, y;
	for (y = 0; y < CHUNK_SIZE; ++y) {
		for (x = 0; x < CHUNK_SIZE; ++x) {
			if (a->tiles[x][y] != b->tiles[x][y]) {
				printf("@(%d, %d), tiles mismatch: %d != %d\n",
					x, y, a->tiles[x][y], b->tiles[x][y]);
				return false;
			} else if (fabs(a->heights[x][y] - b->heights[x][y]) > err) {
				printf("@(%d, %d), heights mismatch: %f != %f\n",
					x, y, a->heights[x][y], b->heights[x][y]);
				return false;
			}
		}
	}

	return true;
}

#define CNT 512
#define BLEN (800 * CNT)

int32_t
main(int32_t argc, const char *const argv[])
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	srand(ts.tv_nsec);
	log_init();
	log_level = ll_debug;

	uint8_t *buf = z_calloc(BLEN, 1);
	struct chunk *c = z_calloc(CNT, sizeof(struct chunk)),
		     *u = z_calloc(CNT, sizeof(struct chunk));
	uint32_t i, len;

	L(log_misc, "shuffling");
	for (i = 0; i < CNT; ++i) {
		randomize_chunk(&c[i]);
	}

	L(log_misc, "packing");
	clock_gettime(CLOCK_MONOTONIC, &ts);
	long ts_pack_start = ts.tv_nsec;

	len = 0;
	for (i = 0; i < CNT; ++i) {
		randomize_chunk(&c[i]);
		/* print_chunk(&c[i]); */
		len += pack_chunk(&c[i], &buf[len], BLEN - len);
	}

	clock_gettime(CLOCK_MONOTONIC, &ts);
	long ts_unpack_start = ts.tv_nsec;

	L(log_misc, "unpacking");

	len = 0;
	for (i = 0; i < CNT; ++i) {
		len += unpack_chunk(&u[i], &buf[len], BLEN - len);
		/* print_chunk(&u[i]); */
	}

	clock_gettime(CLOCK_MONOTONIC, &ts);
	long ts_fin = ts.tv_nsec;

	L(log_misc, "raw: %ld / compressed: %d, ratio: %0.2f",
		sizeof(struct chunk) * CNT,
		len,
		(float)(sizeof(struct chunk) * CNT) / (float)len);
	L(log_misc, "pack: %.1fns/chunk | unpack: %.1fns/chunk",
		(ts_unpack_start - ts_pack_start) / (float)len,
		(ts_fin - ts_unpack_start) / (float)len
		);


	for (i = 0; i < CNT; ++i) {
		if (!chunks_eql(&c[i], &u[i], 0.1)) {
			return 1;
		}
	}
}
