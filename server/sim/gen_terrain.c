#include <math.h>

#include "server/sim/gen_terrain.h"
#include "server/sim/terrain.h"
#include "shared/math/delaunay.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

void
draw_point(struct chunks *chunks, enum tile t, const struct pointf *cc)
{
	struct point cci = { (int)cc->x, (int)cc->y };
	if (get_tile_at(chunks, &cci) == 0) {
		update_tile(chunks, &cci, t);
	}
}

void
draw_edge(struct chunks *chunks, enum tile t, const struct tg_edge *e)
{
	const struct pointf *p = e->a, *q = e->b;
	struct pointf r = *p;

	float dx = q->x - p->x, dy = q->y - p->y,
	      l = fabs(dx) > fabs(dy) ? fabs(dx) : fabs(dy), i;

	dx /= l;
	dy /= l;

	for (i = 0; i < l; ++i) {
		draw_point(chunks, t, &r);
		r.x += dx;
		r.y += dy;
	}
}

void
gen_terrain(struct chunks *chunks, uint32_t width, uint32_t height, uint32_t points)
{
	uint32_t i;
	struct trigraph tg = { 0 };
	struct pointf p;

	trigraph_init(&tg);
	p = (struct pointf){ 0, 0 };
	darr_push(tg.points, &p);
	p = (struct pointf){ 0, height };
	darr_push(tg.points, &p);
	p = (struct pointf){ width, height };
	darr_push(tg.points, &p);
	p = (struct pointf){ width, 0 };
	darr_push(tg.points, &p);
	tg_scatter(&tg, width, height, points);

	delaunay(&tg);

	L("tris: %ld, edges: %ld", hdarr_len(tg.tris), hdarr_len(tg.edges));

	/* draw results */
	for (i = 0; i < darr_len(tg.points); ++i) {
		struct pointf *p = darr_get(tg.points, i);

		draw_point(chunks, tile_forest, p);
	}

	for (i = 0; i < hdarr_len(tg.edges); ++i) {
		struct tg_edge *e = darr_get(hdarr_darr(tg.edges), i);

		draw_edge(chunks, tile_peak, e);
	}
}
