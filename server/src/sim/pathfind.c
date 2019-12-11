#define _DEFAULT_SOURCE
#include <limits.h>
#include <stdlib.h>

#include "constants/tile_chars.h"
#include "pathfind.h"
#include "sim/chunk.h"
#include "terrain.h"
#include "util/log.h"

static enum tile tile_at_point(struct hash *chnks, struct point *p)
{
	struct point np = nearest_chunk(p), rp = point_sub(p, &np);

	return get_chunk(chnks, &np)->tiles[rp.x][rp.y];
}

static int valid_move(enum tile t)
{
	return t <= tile_forest;
}

void meander(struct hash *chunks, struct point *s)
{
	struct point *pps[4], ps[] = {
		{ s->x + 1, s->y },
		{ s->x - 1, s->y },
		{ s->x, s->y + 1 },
		{ s->x, s->y - 1 },
	};
	int i, poss = 0;

	for (i = 0; i < 4; i++)
		if (valid_move(tile_at_point(chunks, &ps[i])))
			pps[poss++] = &ps[i];

	*s = *pps[random() % poss];
}

#define RNG 8
#define RNGS RNG * RNG

struct node {
	struct node *adj[4];
	struct node *prev;
	struct point pos;
	enum tile t;
	int acnt;
	int wd;
	int wh;
	int v;
};

static struct node *make_graph(struct hash *chunks, const struct point *s, struct point *e, struct node g[RNG][RNG])
{
	struct chunk *ck;
	struct point op = { s->x - (RNG / 2), 0 }, rp, np;
	struct node *sp = NULL;
	int i, j, ai, opy = s->y - (RNG / 2);

	for (i = 0; i < RNG; i++, op.x++)
		for (j = 0, op.y = opy; j < RNG; j++, op.y++) {
			ai = 0;

			np = nearest_chunk(&op);
			ck = get_chunk(chunks, &np);
			rp = point_sub(&op, &ck->pos);

			g[i][j].t = ck->tiles[rp.x][rp.y];

			if (!valid_move(g[i][j].t)) {
				g[i][j].wh = g[i][j].wd = INT_MAX;
				continue;
			}

			if (i > 0) {
				g[i][j].adj[ai] = &g[i - 1][j];
				ai++;
			}
			if (i < RNG - 1) {
				g[i][j].adj[ai] = &g[i + 1][j];
				ai++;
			}
			if (j > 0) {
				g[i][j].adj[ai] = &g[i][j - 1];
				ai++;
			}
			if (j < RNG - 1) {
				g[i][j].adj[ai] = &g[i][j + 1];
				ai++;
			}

			g[i][j].acnt = ai;
			g[i][j].pos = op;
			g[i][j].v = 0;
			g[i][j].prev = NULL;

			if (op.x == s->x && op.y == s->y) {
				sp = &g[i][j];
				sp->wd = 0;
				sp->wh = square_dist(e, &op);
			} else {
				g[i][j].wh = g[i][j].wd = INT_MAX;
			}
		}

	return sp;
}

static struct node *dijkstra(struct node g[RNG][RNG], struct node *n, struct point *e)
{
	int i, j, mindist, fuv;

	while (1) {
		mindist = INT_MAX;
		fuv = 0;

		for (i = 0; i < RNG; i++) {
			for (j = 0; j < RNG; j++) {
				if (g[i][j].v || !valid_move(g[i][j].t)) {
					continue;
				} else if (g[i][j].wh < mindist) {
					fuv = 1;
					mindist = g[i][j].wh;
					n = &g[i][j];
				}
			}
		}

		if (!fuv)
			break;

		n->v = 1;

		for (i = 0; i < n->acnt; i++)
			if (valid_move(n->adj[i]->t) && n->wd + 1 < n->adj[i]->wd) {
				n->adj[i]->wd = n->wd + 1;
				n->adj[i]->wh = n->adj[i]->wd + square_dist(&n->adj[i]->pos, e);
				n->adj[i]->prev = n;
			}
	}

	mindist = INT_MAX;
	for (i = 0; i < RNG; i++) {
		for (j = 0; j < RNG; j++) {
			if (!valid_move(g[i][j].t))
				continue;

			if (g[i][j].pos.x == e->x && g[i][j].pos.y == e->y) {
				return &g[i][j];
			} else if (g[i][j].wh < mindist) {
				mindist = g[i][j].wh;
				n = &g[i][j];
			}
		}
	}

	return n;
}

void pathfind(struct hash *chunks, struct point *s, struct point *f)
{
	struct node g[RNG][RNG], *sn, *dn;

	L("pathfinding (%d, %d) -> (%d, %d)", s->x, s->y, f->x, f->y);

	sn = make_graph(chunks, s, f, g);
	dn = dijkstra(g, sn, f);
	L("dn: (%d, %d)", dn->pos.x, dn->pos.y);

	int i, j;
	for (j = 0; j < RNG; j++) {
		for (i = 0; i < RNG; i++) {
			if (&g[i][j] == sn)
				printf("         %c ", sn == dn ? '$' : '@');
			else if (&g[i][j] == dn)
				printf("         D ");
			else if (g[i][j].wd == INT_MAX)
				printf("         _ ");
			else
				printf("%10d ",  g[i][j].wh);

			printf("%c ",  tile_chars[g[i][j].t]);
		}

		printf("\n");
	}

	while (dn->prev != sn) {
		if (dn->prev == NULL) {
			L("failed to find route");
			meander(chunks, s);

			return;
		}
		dn = dn->prev;
	}

	*s = dn->pos;
}
