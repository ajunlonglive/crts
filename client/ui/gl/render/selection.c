#include "posix.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#include "client/client.h"
#include "client/ui/gl/globals.h"
#include "client/ui/gl/render/chunks.h"
#include "client/ui/gl/render/selection.h"
#include "client/ui/gl/shader.h"
#include "shared/constants/globals.h"
#include "shared/sim/tiles.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "shared/util/util.h"
#include "tracy.h"

typedef float highlight_block[8][2][4];

static struct darr selection_data = { 0 };
static struct darr draw_counts = { 0 };
static struct darr draw_indices = { 0 };
static struct darr draw_baseverts = { 0 };
static struct darr lines = { 0 };

size_t sel_indices_len = 36;
static const uint32_t sel_indices[36] = {
	7, 5, 4, 6, 7, 4,  // top
	5, 7, 3, 1, 5, 3,  // east
	6, 2, 7, 2, 3, 7,  // south
	6, 4, 0, 2, 6, 0,  // west
	4, 5, 1, 0, 4, 1,  // north
	0, 1, 2, 1, 3, 2,  // bottom
};

void *null_pointer = 0;

enum sel_uniform {
	su_pulse = UNIFORM_START_RP_FINAL,
};

struct shader sel_shader;
struct shader line_shader;

bool
render_world_setup_selection(void)
{
	struct shader_spec sel_spec = {
		.src = {
			[rp_final] = {
				{ "selection.vert", GL_VERTEX_SHADER },
				{ "basic.frag", GL_FRAGMENT_SHADER },
			}
		},
		.uniform = { [rp_final] = { { su_pulse, "pulse" } } },
		.attribute = { { { 4, GL_FLOAT, bt_vbo }, { 4, GL_FLOAT, bt_vbo } } },
		.static_data = {
			{ sel_indices, sizeof(uint32_t) * sel_indices_len, bt_ebo },
		},
		.uniform_blacklist = {
			[rp_final] = 0xffff & ~(1 << duf_viewproj
						| 1 << duf_clip_plane),
		}
	};

	if (!shader_create(&sel_spec, &sel_shader)) {
		return false;
	}

	struct shader_spec line_spec = {
		.src = {
			[rp_final] = {
				{ "points.vert", GL_VERTEX_SHADER },
				{ "basic.frag", GL_FRAGMENT_SHADER },
			}
		},
		.attribute = {
			{ { 3, GL_FLOAT, bt_vbo, true }, { 3, GL_FLOAT, bt_vbo } }
		},
		.uniform_blacklist = {
			[rp_final] = 0xffff & ~(1 << duf_viewproj
						| 1 << duf_clip_plane),
		}
	};

	if (!shader_create(&line_spec, &line_shader)) {
		return false;
	}

	darr_init(&selection_data, sizeof(highlight_block));
	darr_init(&draw_counts, sizeof(GLsizei));
	darr_init(&draw_indices, sizeof(GLvoid *));
	darr_init(&draw_baseverts, sizeof(GLint));
	darr_init(&lines, sizeof(float) * 6);

	return true;
}

static void
setup_hightlight_block(vec4 clr, struct point *p, float z, float s)
{
	float sh = s / 2;
	highlight_block sel = {
		{ { p->x - sh, z - sh, p->y - sh }, { clr[0], clr[1], clr[2], clr[3] }, },
		{ { p->x + sh, z - sh, p->y - sh }, { clr[0], clr[1], clr[2], clr[3] }, },
		{ { p->x - sh, z - sh, p->y + sh }, { clr[0], clr[1], clr[2], clr[3] }, },
		{ { p->x + sh, z - sh, p->y + sh }, { clr[0], clr[1], clr[2], clr[3] }, },
		{ { p->x - sh, z + sh, p->y - sh }, { clr[0], clr[1], clr[2], clr[3] }, },
		{ { p->x + sh, z + sh, p->y - sh }, { clr[0], clr[1], clr[2], clr[3] }, },
		{ { p->x - sh, z + sh, p->y + sh }, { clr[0], clr[1], clr[2], clr[3] }, },
		{ { p->x + sh, z + sh, p->y + sh }, { clr[0], clr[1], clr[2], clr[3] }, },
	};

	darr_push(&draw_counts, &sel_indices_len);
	darr_push(&draw_indices, &null_pointer);

	size_t basevert = darr_len(&selection_data) * 8;
	darr_push(&draw_baseverts, &basevert);

	darr_push(&selection_data, sel);
}

static bool
trace_cursor_check_terrain_intersection(struct gl_ui_ctx *ctx, const float *origin, const float *dir, const struct point *p)
{
	chunk_mesh *ck;
	struct point np = nearest_chunk(p);

	if (!(ck = hdarr_get(&ctx->chunk_meshes, &np))) {
		return 0.0f;
	}

	np = point_sub(p, &np);

	uint32_t idx = (np.x + np.y * CHUNK_SIZE) * 2;

	const uint32_t *t;

	t = &chunk_indices[idx * 3];
	if (ray_intersects_tri(origin, dir, (*ck)[t[0]].pos, (*ck)[t[1]].pos, (*ck)[t[2]].pos)) {
		return true;
	}

	t = &chunk_indices[(idx + 1) * 3];
	if (ray_intersects_tri(origin, dir, (*ck)[t[0]].pos, (*ck)[t[1]].pos, (*ck)[t[2]].pos)) {
		return true;
	}

	return false;
}

static bool
trace_cursor_check_ent_intersection(struct ent *e, const float *origin, const float *dir, vec3 p)
{
	memcpy(p, e->real_pos,  sizeof(vec3));
	const float block[8][3] = {
		{ p[0] - 0.5f, p[1] - 0.5f, p[2] - 0.5f },
		{ p[0] + 0.5f, p[1] - 0.5f, p[2] - 0.5f },
		{ p[0] - 0.5f, p[1] - 0.5f, p[2] + 0.5f },
		{ p[0] + 0.5f, p[1] - 0.5f, p[2] + 0.5f },
		{ p[0] - 0.5f, p[1] + 0.5f, p[2] - 0.5f },
		{ p[0] + 0.5f, p[1] + 0.5f, p[2] - 0.5f },
		{ p[0] - 0.5f, p[1] + 0.5f, p[2] + 0.5f },
		{ p[0] + 0.5f, p[1] + 0.5f, p[2] + 0.5f },
	}, *t0, *t1, *t2;

	static const float faces[6][3] = {
		{ 0, +1, 0, }, // top
		{ 1, 0, 0, },  // east
		{ 0, 0, 1, },  // south
		{ -1, 0, 0, }, // west
		{ 0, 0, -1, }, // north
		{ 0, -1, 0, }, // bottom
	};

	vec4 dir4 = { dir[0], dir[1], dir[2], 1 };
	vec_add(dir4, origin);

	uint8_t i;
	for (i = 0; i < sel_indices_len / 3; ++i) {
		t0 = block[sel_indices[(i * 3) + 0]];
		t1 = block[sel_indices[(i * 3) + 1]];
		t2 = block[sel_indices[(i * 3) + 2]];

		vec4 plane;
		make_plane(t0, t1, t2, plane);
		if (vec4_dot(plane, dir4) < 0) {
			continue;
		}

		if (ray_intersects_tri(origin, dir, t0, t1, t2)) {
			vec_add(p, faces[i / 2]);
			return true;
		}
	}

	return false;
}

static bool
trace_cursor_check_point(struct gl_ui_ctx *ctx, struct client *cli,
	const float *behind, const float *dir, const int32_t p[3])
{
	struct point3d key = { p[0], p[1], p[2] };
	vec3 cursor = { 0 };

	struct ent **e;
	if ((e = (struct ent **)hash_get(&cli->ents, &key))
	    && trace_cursor_check_ent_intersection(*e, behind, dir, cursor)) {
		cli->cursorf.x = cursor[0];
		cli->cursorf.y = cursor[2];
		cli->cursor = (struct point) { cursor[0], cursor[2] };
		cli->cursor_z = cursor[1];
		return true;
	} else if (trace_cursor_check_terrain_intersection(ctx, behind, dir, &(struct point) { p[0], p[2] })) {
		cli->cursorf.x = p[0];
		cli->cursorf.y = p[2];
		cli->cursor.x = p[0];
		cli->cursor.y = p[2];
		cli->cursor_z = p[1];
		return true;
	}

	return false;
}

static void
trace_cursor_to_world(struct gl_ui_ctx *ctx, struct client *cli)
{
	static vec4 cam_pos, cam_tgt;
	if (!cam.unlocked) {
		memcpy(cam_pos, cam.pos, sizeof(vec4));
		memcpy(cam_tgt, cam.tgt, sizeof(vec4));
	}

	float wh = tanf(cam.fov / 2.0f),
	      ww = (wh / (float)ctx->win->sc_height) * (float)ctx->win->sc_width;

	vec4 right = { 0, 1, 0 }, up,
	     dir = { cam_tgt[0], cam_tgt[1], cam_tgt[2], cam_tgt[3] };
	vec_normalize(dir);
	vec_cross(right, dir);
	vec_normalize(right);
	memcpy(up, dir, sizeof(vec4));
	vec_cross(up, right);

	float cx = (ctx->sc_cursor.x * 2.0f - 1.0f) * ww,
	      cy = (ctx->sc_cursor.y * 2.0f - 1.0f) * wh;

	vec_scale(up, -cy);
	vec_scale(right, cx);

	float curs[3] = { 0 };
	vec_add(curs, cam_pos);
	vec_add(curs, up);
	vec_add(curs, right);

	float behind[3] = { 0 };
	vec_add(behind, cam_pos);
	vec_scale(dir, 1.0f);
	vec_add(behind, dir);

	memcpy(dir, curs, sizeof(float) * 3);
	vec_sub(dir, behind);
	vec_normalize(dir);

	/* draw ray */
	/* { */
	/* 	vec3 long_dir = { 0 }; */
	/* 	vec_add(long_dir, dir); */
	/* 	vec_scale(long_dir, 1000); */
	/* 	vec_add(long_dir, behind); */
	/* 	darr_push(&lines, (float []) { behind[0], behind[1], behind[2], 1, 1, 1 }); */
	/* 	darr_push(&lines, (float []) { long_dir[0], long_dir[1], long_dir[2], 1, 1, 1 }); */
	/* } */

	/* 3D DDA based on https://lodev.org/cgtutor/raycasting.html
	 * and http://www.cse.yorku.ca/~amana/research/grid.pdf
	 * */

	int32_t map[] = { behind[0], behind[1], behind[2] };
	vec3 delta_dist = { fabsf(1 / dir[0]), fabsf(1 / dir[1]), fabsf(1 / dir[2]) };
	int32_t steps[3] = { 0 };
	vec3 side_dist = { 0 };

	uint32_t i;
	for (i = 0; i < 3; ++i) {
		if (dir[i] < 0) {
			steps[i] = -1;
			side_dist[i] = (0.5f + behind[i] - map[i]) * delta_dist[i];
		} else {
			steps[i] = 1;
			side_dist[i] = (map[i] + 0.5f - behind[i]) * delta_dist[i];
		}
	}

	uint32_t limit = 0;
	while (true) {
		if (trace_cursor_check_point(ctx, cli, behind, dir, map)) {
			break;
		}

		if (++limit > 1000) {
			/* prevent an infinite loop if the ray hits nothing
			 * (e.g. due to world loading)
			 */
			break;
		}

		if (side_dist[0] < side_dist[1]) {
			if (side_dist[0] < side_dist[2]) {
				map[0] += steps[0];
				side_dist[0] += delta_dist[0];
			} else {
				map[2] += steps[2];
				side_dist[2] += delta_dist[2];
			}
		} else {
			if (side_dist[1] < side_dist[2]) {
				map[1] += steps[1];
				side_dist[1] += delta_dist[1];
			} else {
				map[2] += steps[2];
				side_dist[2] += delta_dist[2];
			}
		}
	}
}

void
render_selection_setup_frame(struct client *cli, struct gl_ui_ctx *ctx)
{
	TracyCZoneAutoS;

	darr_clear(&selection_data);
	darr_clear(&draw_counts);
	darr_clear(&draw_indices);
	darr_clear(&draw_baseverts);
	darr_clear(&lines);

	trace_cursor_to_world(ctx, cli);

	vec4 clr = { 0, 1, 1, 0.8 };
	setup_hightlight_block(clr, &cli->cursor, cli->cursor_z, 1.0f);

	glBindBuffer(GL_ARRAY_BUFFER, sel_shader.buffer[bt_vbo]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(highlight_block) * darr_len(&selection_data),
		darr_raw_memory(&selection_data), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, line_shader.buffer[bt_vbo]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * 6 * darr_len(&lines),
		darr_raw_memory(&lines), GL_DYNAMIC_DRAW);

	TracyCZoneAutoE;
}

void
render_selection(struct client *cli, struct gl_ui_ctx *ctx)
{
	assert(ctx->pass == rp_final);

	glUseProgram(sel_shader.id[rp_final]);
	shader_check_def_uni(&sel_shader, ctx);
	glBindVertexArray(sel_shader.vao[rp_final][0]);

	float pulse = (sinf(ctx->pulse_ms / 100.0f) + 1.0f) / 2.0f;
	glUniform1fv(sel_shader.uniform[rp_final][su_pulse], 1, &pulse);

	glMultiDrawElementsBaseVertex(
		GL_TRIANGLES,
		darr_raw_memory(&draw_counts),
		GL_UNSIGNED_INT,
		darr_raw_memory(&draw_indices),
		darr_len(&selection_data),
		darr_raw_memory(&draw_baseverts));

	glUseProgram(line_shader.id[rp_final]);
	shader_check_def_uni(&line_shader, ctx);
	glBindVertexArray(line_shader.vao[rp_final][0]);
	glPointSize(5);
	glDrawArrays(GL_POINTS, 0, darr_len(&lines));
	glDrawArrays(GL_LINES, 0, darr_len(&lines));
}
