#ifndef SHARED_MATH_TRIGRAPH_H
#define SHARED_MATH_TRIGRAPH_H

#include <stdbool.h>

#include "shared/types/geom.h"

typedef const struct pointf *tg_trikey[3];
typedef const struct pointf *tg_edgekey[2];

struct tg_edge {
	const struct pointf *a, *b;
	tg_trikey adja, adjb;
};

struct tg_tri {
	const struct pointf *a, *b, *c;
	tg_edgekey ab, bc, ac;
	double alpha, beta, gamma;
};

struct trigraph {
	struct darr *points;
	struct hdarr *edges, *tris;
};

const struct tg_tri *tg_get_tri(struct trigraph *g, const struct pointf *a, const struct pointf *b,
	const struct pointf *c);
const struct tg_tri *tg_get_trik(struct trigraph *g, const tg_trikey tk);
const struct tg_edge *tg_get_edgek(struct trigraph *g, const tg_edgekey ek);
double tg_point_angle(const struct tg_tri *t, const struct pointf *p);
double tg_opposite_angle(const struct tg_tri *t, const struct tg_edge *e);
void tg_del_tri(struct trigraph *g, const struct tg_tri *t);
bool tg_edges_eql(const struct tg_edge *a, const tg_edgekey b);
bool tg_tris_eql(const struct tg_tri *a, const tg_trikey b);
void trigraph_init(struct trigraph *tg);
void trigraph_clear(struct trigraph *tg);
void tg_scatter(struct trigraph *tg, uint32_t width, uint32_t height, uint32_t amnt,
	float r);
const struct tg_edge *next_edge(struct trigraph *tg, const struct tg_tri *t,
	const struct tg_edge *cur, const struct pointf *p);

typedef void ((*tg_for_each_adjacent_point_cb)
	      (const struct pointf *p, const struct tg_edge *e, void *_ctx));
void tg_for_each_adjacent_point(struct trigraph *tg, const struct pointf *p,
	const struct tg_edge *e, void *ctx, tg_for_each_adjacent_point_cb cb);
#endif
