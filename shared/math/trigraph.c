/* ??=??(??/??)??'??<??!??>??-??! */
#include "posix.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#include "shared/math/geom.h"
#include "shared/math/rand.h"
#include "shared/math/trigraph.h"
#include "shared/types/darr.h"
#include "shared/types/hash.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"

bool
tg_edges_eql(const struct tg_edge *a, const tg_edgekey b)
{
	return a->a == b[0] && a->b == b[1];
}

bool
tg_tris_eql(const struct tg_tri *a, const tg_trikey b)
{
	return a->a == b[0]
	       && a->b == b[1]
	       && a->c == b[2];
}

static const struct tg_edge *
get_edge(struct trigraph *g, const struct pointf *a, const struct pointf *b)
{
	const struct pointf *kp[2];
	struct tg_edge *ep;

	assert(a != b && a && b );

	if (a < b) {
		kp[0] = a; kp[1] = b;
	} else {
		kp[0] = b; kp[1] = a;
	}

	tg_edgekey ek = { kp[0], kp[1] };

	if (!(ep = hdarr_get(g->edges, ek))) {
		struct tg_edge e = { kp[0], kp[1], NULL, NULL };
		hdarr_set(g->edges, ek, &e);
		ep = hdarr_get(g->edges, ek);
	}

	assert(ep->a && ep->b);

	return ep;
}

const struct tg_edge *
tg_get_edgek(struct trigraph *g, const tg_edgekey ek)
{
	return get_edge(g, ek[0], ek[1]);
}

void
tg_for_each_adjacent_point(struct trigraph *tg, const struct pointf *p,
	const struct tg_edge *e, void *ctx, tg_for_each_adjacent_point_cb cb)
{
	/* uint32_t i; */
	const struct tg_tri *t = hdarr_get(tg->tris, e->adja);
	const struct tg_edge *oe = e;

	do {
		if (p == e->a) {
			cb(e->b, e, ctx);
		} else if (p == e->b) {
			cb(e->a, e, ctx);
		} else {
			assert(false);
		}

		e = next_edge(tg, t, e, p);

		if (tg_tris_eql(t, e->adja)) {
			if (e->adjb[0]) {
				t = hdarr_get(tg->tris, e->adjb);
			}
		} else {
			t = hdarr_get(tg->tris, e->adja);
		}
	} while (e != oe);
}

static void
attach_tri(const struct tg_edge *e, const struct tg_tri *t)
{
	tg_trikey tk = { t->a, t->b, t->c };

	if (!e->adja[0]) {
		memcpy(((struct tg_edge *)e)->adja, tk, sizeof(tg_trikey));
	} else if (!e->adjb[0]) {
		memcpy(((struct tg_edge *)e)->adjb, tk, sizeof(tg_trikey));
	} else {
		assert(false);
	}
}

static void
detach_tri(struct trigraph *g, const struct tg_edge *e, const struct tg_tri *t)
{
	if (tg_tris_eql(t, e->adja)) {
		if (e->adjb[0] == NULL) {
			tg_edgekey ek = { e->a, e->b };
			hdarr_del(g->edges, ek);
			return;
		}

		assert(!tg_tris_eql(t, e->adjb));

		memcpy(((struct tg_edge *)e)->adja, e->adjb, sizeof(tg_trikey));
	} else if (tg_tris_eql(t, e->adjb)) {
		assert(e->adja[0]);
	} else {
		assert(false);
	}

	((struct tg_edge *)e)->adjb[0] = NULL;
}

const struct tg_tri *
tg_get_tri(struct trigraph *g, const struct pointf *a, const struct pointf *b,
	const struct pointf *c)
{
	struct tg_tri t = { 0 }, *tp;

	assert(a && b && c);
	assert(signed_area(a, b, c) != 0.0f);

	if (a < b) {
		t.a = a;
		t.b = b;
	} else {
		t.a = b;
		t.b = a;
	}

	if (t.b > c) {
		t.c = t.b;
		if (t.a > c) {
			t.b = t.a;
			t.a = c;
		} else {
			t.b = c;
		}
	} else {
		t.c = c;
	}

	if (signed_area(t.a, t.b, t.c) < 0) {
		const struct pointf *tmp = t.a;
		t.a = t.b;
		t.b = tmp;
	}

	tg_trikey tk = { t.a, t.b, t.c };

	if (!(tp = hdarr_get(g->tris, tk))) {
		hdarr_set(g->tris, tk, &t);
		tp = hdarr_get(g->tris, tk);

		attach_tri(get_edge(g, t.a, t.b), tp);
		attach_tri(get_edge(g, t.b, t.c), tp);
		attach_tri(get_edge(g, t.a, t.c), tp);

		const struct tg_edge *es[3] = { get_edge(g, t.a, t.b), get_edge(g, t.b, t.c), get_edge(g, t.a, t.c), };

		memcpy(tp->ab, es[0], sizeof(tg_edgekey));
		memcpy(tp->bc, es[1], sizeof(tg_edgekey));
		memcpy(tp->ac, es[2], sizeof(tg_edgekey));

		double slen[] = { fsqdist(t.a, t.b), fsqdist(t.a, t.c), fsqdist(t.b, t.c) },
		       len[] = { sqrt(slen[0]), sqrt(slen[1]), sqrt(slen[2]) };

		tp->alpha = acos(((slen[0] + slen[1]) - slen[2]) / (2 * len[0] * len[1]));
		tp->beta  = acos(((slen[0] + slen[2]) - slen[1]) / (2 * len[0] * len[2]));
		tp->gamma = acos(((slen[1] + slen[2]) - slen[0]) / (2 * len[1] * len[2]));

		assert(fabs(PI - (tp->alpha + tp->beta + tp->gamma)) < 0.0001);
	}

	assert(tp->ab[0] && tp->ab[1] && tp->bc[0] && tp->bc[1] && tp->ac[0]
		&& tp->ac[1]);

	return tp;
}

const struct tg_tri *
tg_get_trik(struct trigraph *g, const tg_trikey tk)
{
	if (!tk[0]) {
		return NULL;
	} else {
		return tg_get_tri(g, tk[0], tk[1], tk[2]);
	}
}

void
tg_del_tri(struct trigraph *g, const struct tg_tri *t)
{
	detach_tri(g, tg_get_edgek(g, t->ab), t);
	detach_tri(g, tg_get_edgek(g, t->bc), t);
	detach_tri(g, tg_get_edgek(g, t->ac), t);

	tg_trikey tk = { t->a, t->b, t->c };
	hdarr_del(g->tris, tk);
	assert(hdarr_get(g->tris, tk) == NULL);
}


const void *
tri_reverse_key(void *_tri)
{
	const struct tg_tri *t = _tri;
	static tg_trikey tk;

	tk[0] = t->a;
	tk[1] = t->b;
	tk[2] = t->c;

	return tk;
}

const void *
edge_reverse_key(void *_edge)
{
	const struct tg_edge *e = _edge;
	static tg_edgekey ek;

	ek[0] = e->a;
	ek[1] = e->b;

	return ek;
}

double
tg_opposite_angle(const struct tg_tri *t, const struct tg_edge *e)
{
	if (tg_edges_eql(e, t->ab)) {
		return t->gamma;
	} else if (tg_edges_eql(e, t->bc)) {
		return t->alpha;
	} else if (tg_edges_eql(e, t->ac)) {
		return t->beta;
	} else {
		assert(false);
	}
}

double
tg_point_angle(const struct tg_tri *t, const struct pointf *p)
{
	if (p == t->a) {
		return t->alpha;
	} else if (p == t->b) {
		return t->beta;
	} else if (p == t->c) {
		return t->gamma;
	} else {
		assert(false);
	}
}

const struct tg_edge *
next_edge(struct trigraph *tg, const struct tg_tri *t,
	const struct tg_edge *cur, const struct pointf *p)
{
	if (tg_edges_eql(cur, t->ab)) {
		if (p == t->a) {
			return hdarr_get(tg->edges, t->ac);
		} else {
			return hdarr_get(tg->edges, t->bc);
		}
	} else if (tg_edges_eql(cur, t->bc)) {
		if (p == t->b) {
			return hdarr_get(tg->edges, t->ab);
		} else {
			return hdarr_get(tg->edges, t->ac);
		}
	} else if (tg_edges_eql(cur, t->ac)) {
		if (p == t->c) {
			return hdarr_get(tg->edges, t->bc);
		} else {
			return hdarr_get(tg->edges, t->ab);
		}
	} else {
		assert(false);
	}
}

void
trigraph_init(struct trigraph *tg)
{
	tg->points = darr_init(sizeof(struct pointf)),
	tg->edges = hdarr_init(2048, sizeof(tg_edgekey), sizeof(struct tg_edge),
		edge_reverse_key);
	tg->tris = hdarr_init(2048, sizeof(tg_trikey), sizeof(struct tg_tri),
		tri_reverse_key);
}

void
trigraph_clear(struct trigraph *tg)
{
	darr_clear(tg->points);
	hdarr_clear(tg->edges);
	hdarr_clear(tg->tris);
}

void
tg_scatter(struct trigraph *tg, uint32_t width, uint32_t height, uint32_t amnt,
	float r)
{
	uint32_t i;
	struct pointf z = { width / 2, height / 2 };
	struct hash *picked = hash_init(2048, 1, sizeof(struct pointf));
	r *= width < height ? width : height;
	r *= r;

	for (i = 0; i < amnt; ++i) {
		struct pointf p;
		struct point q;

		do {
			p = (struct pointf){ rand_uniform(width - 2) + 1,
					     rand_uniform(height - 2) + 1 };
			q = (struct point){ p.x, p.y };
		} while (fsqdist(&z, &p) > r || hash_get(picked, &q));

		hash_set(picked, &q, 1);

		darr_push(tg->points, &p);
	}

	hash_destroy(picked);
}
