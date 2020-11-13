#include "posix.h"

#include <string.h>

#include "client/ui/opengl/render/pathfinding_overlay.h"
#include "client/ui/opengl/shader.h"
#include "shared/pathfind/local.h"
#include "shared/pathfind/pg_node.h"
#include "shared/pathfind/preprocess.h"
#include "shared/util/log.h"

#define GRID_H 1
#define GRID_C 0, 0.05, 0.05
#define ENTRANCE_C 0.2, 0.5, 0
#define EDGE_C 0.5, 0.5, 0

struct shader points_shader = { 0 };
typedef float point[6];
/* TODO: this array is used to draw points and lines, maybe add a new array for
 * points only? */
struct darr *points;

bool
render_world_setup_pathfinding_overlay(void)
{
	struct shader_spec points_spec = {
		.src = {
			[rp_final] = {
				{ "points.vert", GL_VERTEX_SHADER },
				{ "basic.frag", GL_FRAGMENT_SHADER },
			},
		},
		.attribute = {
			{ { 3, GL_FLOAT, bt_vbo, true }, { 3, GL_FLOAT, bt_vbo } }
		},
		.uniform_blacklist = {
			[rp_final] = 0xffff & ~(1 << duf_viewproj) //| 1 << duf_clip_plane),
		},
	};

	if (!shader_create(&points_spec, &points_shader)) {
		return false;
	}

	points = darr_init(sizeof(point));

	return true;
}

static void
setup_chunk_borders(struct chunks *cnks, struct opengl_ui_ctx *ctx)
{
	struct chunk *ck;
	/* struct ag_component *agc; */
	struct point sp = nearest_chunk(&ctx->ref.pos);
	int spy = sp.y,
	    endx = ctx->ref.pos.x + ctx->ref.width,
	    endy = ctx->ref.pos.y + ctx->ref.height;
	/* uint8_t i, j; */

	for (; sp.x < endx; sp.x += CHUNK_SIZE) {
		for (sp.y = spy; sp.y < endy; sp.y += CHUNK_SIZE) {
			if (!(ck = hdarr_get(cnks->hd, &sp))) {
				continue;
			}

#if 0
			agc = hdarr_get(cnks->ag.components, &ck->pos);

			/* perimeter */
			for (i = 0; i < CHUNK_PERIM; ++i) {
				if (!(agc->nodes[i].flags & agn_filled)) {
					continue;
				}

				uint8_t x = ag_component_node_indices[i][0] >> 4,
					y = ag_component_node_indices[i][0] & 15;

				point entrance = { agc->pos.x + x, ck->heights[x][y], agc->pos.y + y, ENTRANCE_C };

				for (j = 0; j < agc->nodes[i].edges; ++j) {
					x = ag_component_node_indices[agc->nodes[i].adjacent[j]][0] >> 4,
					y = ag_component_node_indices[agc->nodes[i].adjacent[j]][0] & 15;

					point edge = { agc->pos.x + x, ck->heights[x][y], agc->pos.y + y, EDGE_C };

					darr_push(points, entrance);
					darr_push(points, edge);
				}
			}
#endif

			/* border */
			point border[] = {
				{ ck->pos.x,      ck->heights[ 0][ 0], ck->pos.y,      GRID_C },
				{ ck->pos.x + 16, ck->heights[15][ 0], ck->pos.y,      GRID_C },
				{ ck->pos.x,      ck->heights[ 0][15], ck->pos.y + 16, GRID_C },
				{ ck->pos.x + 16, ck->heights[15][15], ck->pos.y + 16, GRID_C },
			};

			darr_push(points, border[0]);
			darr_push(points, border[2]);
			darr_push(points, border[2]);
			darr_push(points, border[3]);
			darr_push(points, border[3]);
			darr_push(points, border[1]);
			darr_push(points, border[1]);
			darr_push(points, border[0]);
		}
	}
}

#if 0
static void
draw_ag_component_node(struct chunks *cnks, struct point *cp, uint8_t i)
{
	struct point p = { 0 };
	float clr[3] = { 0 };
	float y;

	struct chunk *ck = hdarr_get(cnks->hd, cp);
	assert(ck);

	struct point q = {
		ag_component_node_indices[i][0] >> 4,
		ag_component_node_indices[i][0] & 15
	};

	p = (struct point){ cp->x + q.x, cp->y + q.y };
	y = ck->heights[q.y][q.x];
	clr[1] = 1;

	point pts[] = { { p.x, y + 1, p.y, clr[0], clr[1], clr[2] } };

	darr_push(points, pts[0]);
}

static void
trace_abstract_path(struct opengl_ui_ctx *ctx, struct chunks *cnks, struct point *goal)
{
	const size_t *v;
	struct ag_component *agc_cur, *agc_prv;

	struct point cp_g = nearest_chunk(goal);
	struct ag_key key = {
		.component = *hdarr_get_i(cnks->ag.components, &cp_g),
		.node = tmp_node
	};

	while ((v = hash_get(cnks->ag.visited, &key))) {
		union ag_val val = { .i = *v };

		if (!val.s.prev.component) {
			break;
		}

		agc_cur = hdarr_get_by_i(cnks->ag.components, key.component);
		agc_prv = hdarr_get_by_i(cnks->ag.components, val.s.prev.component);

		assert(agc_cur && agc_prv);

		/* L("component: prev:%d:(%d, %d), cur:%d:(%d, %d)", key->component, */
		/* 	agc_cur->pos.x, agc_cur->pos.y, val.s.prev.component, */
		/* 	agc_prv->pos.x, agc_prv->pos.y); */

		draw_ag_component_node(cnks, &agc_cur->pos, key.region);
		draw_ag_component_node(cnks, &agc_prv->pos, val.s.prev.region);

		key = val.s.prev;
	}
}
#endif

static void
add_point(struct chunks *cnks, struct point *p)
{
	struct point cp = nearest_chunk(p), rp = point_sub(p, &cp);
	struct chunk *ck = hdarr_get(cnks->hd, &cp);
	float y = 0.0;

	if (ck) {
		y = ck->heights[rp.x][rp.y];

	}

	point g = { p->x, y + 1.2, p->y, 0.5, 0.0, 0.5 };

	darr_push(points, g);
	darr_push(points, g);
}

static void
trace_concrete_path(struct opengl_ui_ctx *ctx, struct chunks *cnks, struct darr *path_points)
{
	uint32_t i;

	for (i = 0; i < darr_len(path_points); ++i) {
		struct point *p = darr_get(path_points, i);
		/* L("%d, %d", p->x, p->y); */
		add_point(cnks, p);
		add_point(cnks, p);
	}
}

void
render_pathfinding_overlay_setup_frame(struct hiface *hf, struct opengl_ui_ctx *ctx)
{
	darr_clear(points);

	setup_chunk_borders(&hf->sim->w->chunks, ctx);

	/* trace_abstract_path(ctx, &hf->sim->w->chunks, &hf->debug_path.goal); */
	trace_concrete_path(ctx, &hf->sim->w->chunks, hf->debug_path.path_points);

	add_point(&hf->sim->w->chunks, &hf->debug_path.goal);
	add_point(&hf->sim->w->chunks, &hf->debug_path.goal);

	uint32_t i;
	for (i = 0; i < darr_len(ctx->debug_hl_points); ++i) {
		add_point(&hf->sim->w->chunks, darr_get(ctx->debug_hl_points, i));
		add_point(&hf->sim->w->chunks, darr_get(ctx->debug_hl_points, i));
	}

	glBindBuffer(GL_ARRAY_BUFFER, points_shader.buffer[bt_vbo]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(point) * darr_len(points),
		darr_raw_memory(points), GL_DYNAMIC_DRAW);
}

void
render_pathfinding_overlay(struct hiface *hf, struct opengl_ui_ctx *ctx)
{
	assert(ctx->pass == rp_final);
	glUseProgram(points_shader.id[rp_final]);
	glBindVertexArray(points_shader.vao[rp_final][0]);
	shader_check_def_uni(&points_shader, ctx);

	glPointSize(15);
	glDrawArrays(GL_POINTS, 0, darr_len(points));
	glDrawArrays(GL_LINES, 0, darr_len(points));
}