#include "posix.h"

#include <string.h>

#include "client/ui/opengl/render/pathfinding_overlay.h"
#include "client/ui/opengl/shader.h"
#include "shared/pathfind/local.h"
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
struct darr points = { 0 };

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

	darr_init(&points, sizeof(point));

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
			if (!(ck = hdarr_get(&cnks->hd, &sp))) {
				continue;
			}

			/* border */
			point border[] = {
				{ ck->pos.x,      ck->heights[ 0][ 0], ck->pos.y,      GRID_C },
				{ ck->pos.x + 16, ck->heights[15][ 0], ck->pos.y,      GRID_C },
				{ ck->pos.x,      ck->heights[ 0][15], ck->pos.y + 16, GRID_C },
				{ ck->pos.x + 16, ck->heights[15][15], ck->pos.y + 16, GRID_C },
			};

			darr_push(&points, border[0]);
			darr_push(&points, border[2]);
			darr_push(&points, border[2]);
			darr_push(&points, border[3]);
			darr_push(&points, border[3]);
			darr_push(&points, border[1]);
			darr_push(&points, border[1]);
			darr_push(&points, border[0]);
		}
	}
}

static void
add_point(struct chunks *cnks, struct point *p)
{
	struct point cp = nearest_chunk(p), rp = point_sub(p, &cp);
	struct chunk *ck = hdarr_get(&cnks->hd, &cp);
	float y = 0.0;

	if (ck) {
		y = ck->heights[rp.x][rp.y];

	}

	point g = { p->x, y + 1.2, p->y, 0.5, 0.0, 0.5 };

	darr_push(&points, g);
	darr_push(&points, g);
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

static enum iteration_result
mark_visited_node(void *_ctx, void *_key, uint64_t val)
{
	struct chunks *cnks = _ctx;
	struct ag_key *key = _key;
	struct ag_component *agc = hdarr_get_by_i(&cnks->ag.components, key->component);
	if (!agc) {
		return ir_cont;
	}

	struct ag_region *rgn = &agc->regions[key->region];
	assert(rgn);

	struct point p = {
		agc->pos.x + (rgn->center >> 4),
		agc->pos.y + (rgn->center & 15)
	};

	add_point(cnks, &p);

	return ir_cont;
}

void
render_pathfinding_overlay_setup_frame(struct client *cli, struct opengl_ui_ctx *ctx)
{
	darr_clear(&points);

	setup_chunk_borders(&cli->world->chunks, ctx);

	hash_for_each_with_keys(&cli->world->chunks.ag.visited, &cli->world->chunks, mark_visited_node);

	trace_concrete_path(ctx, &cli->world->chunks, &cli->debug_path.path_points);

	add_point(&cli->world->chunks, &cli->debug_path.goal);
	add_point(&cli->world->chunks, &cli->debug_path.goal);

	uint32_t i;
	for (i = 0; i < darr_len(&ctx->debug_hl_points); ++i) {
		add_point(&cli->world->chunks, darr_get(&ctx->debug_hl_points, i));
		add_point(&cli->world->chunks, darr_get(&ctx->debug_hl_points, i));
	}

	glBindBuffer(GL_ARRAY_BUFFER, points_shader.buffer[bt_vbo]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(point) * darr_len(&points),
		darr_raw_memory(&points), GL_DYNAMIC_DRAW);
}

void
render_pathfinding_overlay(struct client *cli, struct opengl_ui_ctx *ctx)
{
	assert(ctx->pass == rp_final);
	glUseProgram(points_shader.id[rp_final]);
	glBindVertexArray(points_shader.vao[rp_final][0]);
	shader_check_def_uni(&points_shader, ctx);

	glPointSize(15);
	glDrawArrays(GL_POINTS, 0, darr_len(&points));
	glDrawArrays(GL_LINES, 0, darr_len(&points));
}
