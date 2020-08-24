#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "shared/sim/chunk.h"
#include "shared/util/log.h"

void
chunks_init(struct chunks **cnks)
{
	if (*cnks == NULL) {
		*cnks = calloc(1, sizeof(struct chunks));
	} else {
		memset(*cnks, 0, sizeof(struct chunks));
	}

	(*cnks)->hd = hdarr_init(4096, sizeof(struct point), sizeof(struct chunk), NULL);
#ifdef CRTS_SERVER
	(*cnks)->functional_tiles = hash_init(256, 1, sizeof(struct point));
	(*cnks)->functional_tiles_buf = hash_init(256, 1, sizeof(struct point));
#endif
}

void
chunks_destroy(struct chunks *cnks)
{
	hdarr_destroy(cnks->hd);

#ifdef CRTS_SERVER
	hash_destroy(cnks->functional_tiles);
	hash_destroy(cnks->functional_tiles_buf);
#endif

	free(cnks);
}

static struct chunk *
full_init_chunk(struct chunks *cnks, const struct point *p)
{
	struct chunk c = { 0 }, *cp = &c;

	uint32_t i;
	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		((float *)c.heights)[i] = -5;
	}

	c.pos = *p;
#ifdef CRTS_SERVER
	c.last_touched = cnks->chunk_date;
#endif

	hdarr_set(cnks->hd, p, cp);

	cp = hdarr_get(cnks->hd, p);

	return hdarr_get(cnks->hd, p);
}

static struct chunk *
get_chunk_no_gen(struct chunks *cnks, const struct point *p)
{
	const struct chunk *cnk;

	if ((cnk = hdarr_get(cnks->hd, p)) == NULL) {
		cnk = full_init_chunk(cnks, p);
	}

	assert(cnk->pos.x == p->x && cnk->pos.y == p->y);

	return (struct chunk *)cnk;
}

struct point
nearest_chunk(const struct point *p)
{
	return point_mod(p, CHUNK_SIZE);
}

struct chunk *
get_chunk(struct chunks *cnks, const struct point *p)
{
	struct chunk *c = get_chunk_no_gen(cnks, p);

	assert(!(p->x % CHUNK_SIZE) && !(p->y % CHUNK_SIZE));

	return c;
}

struct chunk *
get_chunk_at(struct chunks *cnks, const struct point *p)
{
	struct point np = nearest_chunk(p);

	return get_chunk(cnks, &np);
}

