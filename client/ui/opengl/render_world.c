#include "posix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "client/ui/opengl/color_cfg.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/render_world.h"
#include "client/ui/opengl/solids.h"
#include "client/ui/opengl/ui.h"
#include "client/ui/opengl/winutil.h"
#include "shared/constants/blueprints.h"
#include "shared/constants/globals.h"
#include "shared/math/linalg.h"
#include "shared/sim/ent.h"
#include "shared/types/darr.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

#define MAX_RENDERED_CHUNKS 512
#define MESH_DIM (CHUNK_SIZE + 1)

struct chunk_info {
	float pos[3];
	float norm[3];
	float type;
};

typedef struct chunk_info chunk_mesh[MESH_DIM * MESH_DIM];

static struct {
	uint32_t id;
	uint32_t vao, vbo;
	uint32_t view, proj, view_pos, positions, types, colors;
} s_ent = { 0 };

static bool
render_world_setup_ents(void)
{
	struct shader_src src[] = {
		{ "client/ui/opengl/shaders/ents.vert", GL_VERTEX_SHADER },
		{ "client/ui/opengl/shaders/world.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!link_shaders(src, &s_ent.id)) {
		return false;
	}

	s_ent.view      = glGetUniformLocation(s_ent.id, "view");
	s_ent.proj      = glGetUniformLocation(s_ent.id, "proj");
	s_ent.positions = glGetUniformLocation(s_ent.id, "positions");
	s_ent.types     = glGetUniformLocation(s_ent.id, "types");
	s_ent.colors    = glGetUniformLocation(s_ent.id, "colors");
	s_ent.view_pos  = glGetUniformLocation(s_ent.id, "view_pos");

	glUseProgram(s_ent.id);

	glGenVertexArrays(1, &s_ent.vao);
	glGenBuffers(1, &s_ent.vbo);

	glBindVertexArray(s_ent.vao);
	glBindBuffer(GL_ARRAY_BUFFER, s_ent.vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * solid_cube.len,
		solid_cube.verts, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
		(void *)0);
	glEnableVertexAttribArray(0);

	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
		(void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// send colors
	glUniform4fv(s_ent.colors, extended_ent_type_count, (float *)colors.ent);

	return true;
}

static struct {
	uint32_t id;
	uint32_t vao, vbo, ebo;
	uint32_t view, proj, view_pos, colors;
	uint32_t count;
	GLsizei draw_counts[MAX_RENDERED_CHUNKS];
	const GLvoid *draw_indices[MAX_RENDERED_CHUNKS];
	GLint draw_baseverts[MAX_RENDERED_CHUNKS];
	struct hdarr *hd;
} s_chunk = { 0 };

bool
render_world_setup_chunks(void)
{
	struct shader_src src[] = {
		{ "client/ui/opengl/shaders/chunks.vert", GL_VERTEX_SHADER },
		{ "client/ui/opengl/shaders/world.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!link_shaders(src, &s_chunk.id)) {
		return false;
	}

	glUseProgram(s_chunk.id);

	s_chunk.view      = glGetUniformLocation(s_chunk.id, "view");
	s_chunk.proj      = glGetUniformLocation(s_chunk.id, "proj");
	s_chunk.view_pos  = glGetUniformLocation(s_chunk.id, "view_pos");
	s_chunk.colors    = glGetUniformLocation(s_chunk.id, "colors");

	glGenVertexArrays(1, &s_chunk.vao);
	glGenBuffers(1, &s_chunk.vbo);
	glGenBuffers(1, &s_chunk.ebo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_chunk.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(uint16_t) * CHUNK_INDICES_LEN, chunk_indices,
		GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, s_chunk.vbo);

	glBindVertexArray(s_chunk.vao);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct chunk_info),
		(void *)0);
	glEnableVertexAttribArray(0);

	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct chunk_info),
		(void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// type attribute
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(struct chunk_info),
		(void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	s_chunk.hd = hdarr_init(2048, sizeof(struct point), sizeof(chunk_mesh), NULL);

	glUniform4fv(s_chunk.colors, tile_count, (float *)colors.tile);

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(chunk_mesh) * MAX_RENDERED_CHUNKS,
		NULL, GL_DYNAMIC_DRAW);

	return true;
}

#define MAX_RENDERED_HL 512
typedef float highlight_block[8][3][3];

static struct {
	uint32_t id;
	uint32_t vao, vbo, ebo;
	uint32_t view, proj, view_pos;
	uint32_t count;
	GLsizei draw_counts[MAX_RENDERED_HL];
	const GLvoid *draw_indices[MAX_RENDERED_HL];
	GLint draw_baseverts[MAX_RENDERED_HL];
} s_selection = { 0 };

static const uint8_t sel_indices[] = {
	1, 3, 0,
	3, 2, 0,
	1, 5, 3,
	5, 7, 3,
	1, 5, 0,
	5, 4, 0,
	2, 6, 0,
	6, 4, 0,
	3, 7, 2,
	7, 6, 2
};

bool
render_world_setup_selection(void)
{
	struct shader_src src[] = {
		{ "client/ui/opengl/shaders/selection.vert", GL_VERTEX_SHADER },
		{ "client/ui/opengl/shaders/world.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!link_shaders(src, &s_selection.id)) {
		return false;
	}

	s_selection.view      = glGetUniformLocation(s_selection.id, "view");
	s_selection.proj      = glGetUniformLocation(s_selection.id, "proj");
	s_selection.view_pos  = glGetUniformLocation(s_selection.id, "view_pos");

	glGenVertexArrays(1, &s_selection.vao);
	glGenBuffers(1, &s_selection.vbo);
	glGenBuffers(1, &s_selection.ebo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_selection.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(sel_indices), sel_indices,
		GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, s_selection.vbo);

	glBindVertexArray(s_selection.vao);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
		(void *)0);
	glEnableVertexAttribArray(0);

	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
		(void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
		(void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(highlight_block) * MAX_RENDERED_CHUNKS,
		NULL, GL_DYNAMIC_DRAW);

	return true;
}

bool
render_world_setup(void)
{
	return render_world_setup_ents()
	       && render_world_setup_chunks()
	       && render_world_setup_selection();
}

void
render_world_teardown(void)
{
	hdarr_destroy(s_chunk.hd);
}

void
update_world_viewport(mat4 mproj)
{
	glUseProgram(s_ent.id);
	glUniformMatrix4fv(s_ent.proj, 1, GL_TRUE, (float *)mproj);

	glUseProgram(s_chunk.id);
	glUniformMatrix4fv(s_chunk.proj, 1, GL_TRUE, (float *)mproj);

	glUseProgram(s_selection.id);
	glUniformMatrix4fv(s_selection.proj, 1, GL_TRUE, (float *)mproj);
}

static void
setup_chunks(struct chunks *cnks, struct opengl_ui_ctx *ctx)
{
	struct chunk *ck, *rck, *bck, *cck;
	struct point sp = nearest_chunk(&ctx->ref.pos), adjp;
	int spy = sp.y,
	    endx = ctx->ref.pos.x + ctx->ref.width,
	    endy = ctx->ref.pos.y + ctx->ref.height,
	    x, y;
	enum tile t;
	uint16_t i;
	float h;
	s_chunk.count = 0;

	chunk_mesh mesh = { 0 }, *draw_mesh;

	for (; sp.x < endx; sp.x += CHUNK_SIZE) {
		for (sp.y = spy; sp.y < endy; sp.y += CHUNK_SIZE) {
			if ((draw_mesh = hdarr_get(s_chunk.hd, &sp))) {
				goto draw_chunk_mesh;
			} else if (!(ck = hdarr_get(cnks->hd, &sp))) {
				continue;
			}

			adjp = sp;
			adjp.x += CHUNK_SIZE;
			rck = hdarr_get(cnks->hd, &adjp);

			adjp.y += CHUNK_SIZE;
			cck = hdarr_get(cnks->hd, &adjp);

			adjp.x -= CHUNK_SIZE;
			bck = hdarr_get(cnks->hd, &adjp);

			for (y = 0; y < MESH_DIM; ++y) {
				for (x = 0; x < MESH_DIM; ++x) {
					t = h = 0;
					if (x >= CHUNK_SIZE && y >= CHUNK_SIZE) {
						if (cck) {
							t = cck->tiles[0][0];
							h = cck->heights[0][0];
						}
					} else if (x >= CHUNK_SIZE) {
						if (rck) {
							t = rck->tiles[0][y];
							h = rck->heights[0][y];
						}
					} else if (y >= CHUNK_SIZE) {
						if (bck) {
							t = bck->tiles[x][0];
							h = bck->heights[x][0];
						}
					} else {
						t = ck->tiles[x][y];
						h = ck->heights[x][y];
					}

					i = y * MESH_DIM + x;

					mesh[i].pos[0] = ck->pos.x + x - 0.5;
					mesh[i].pos[1] = h;
					mesh[i].pos[2] = ck->pos.y + y - 0.5;

					mesh[i].norm[0] = 0;
					mesh[i].norm[1] = 0;
					mesh[i].norm[2] = 0;

					/* TODO: figure out how to do this
					 * without this dumb cast.  Basically
					 * when I try to use a uint the value
					 * gets mangled on its way to the gpu
					 * */
					mesh[i].type = (float)t;
				}
			}

			vec4 a = { 0 }, b = { 0 }, c = { 0 };

			for (x = 0; x < CHUNK_INDICES_LEN; x += 6) {
				memcpy(a, mesh[chunk_indices[x + 0]].pos, sizeof(float) * 3);
				memcpy(b, mesh[chunk_indices[x + 1]].pos, sizeof(float) * 3);
				memcpy(c, mesh[chunk_indices[x + 2]].pos, sizeof(float) * 3);

				vec4_sub(b, a);
				vec4_sub(c, a);
				vec4_cross(b, c);

				memcpy(mesh[chunk_indices[x + 2]].norm, b, sizeof(float) * 3);
			}

			hdarr_set(s_chunk.hd, &ck->pos, mesh);
			draw_mesh = &mesh;

draw_chunk_mesh:
			glBufferSubData(GL_ARRAY_BUFFER,
				s_chunk.count * sizeof(chunk_mesh),
				sizeof(chunk_mesh),
				*draw_mesh);

			s_chunk.draw_counts[s_chunk.count] = CHUNK_INDICES_LEN;
			((GLvoid **)s_chunk.draw_indices)[s_chunk.count] = (void *)0;
			s_chunk.draw_baseverts[s_chunk.count] = s_chunk.count
								* MESH_DIM * MESH_DIM;

			if (++s_chunk.count >= MAX_RENDERED_CHUNKS) {
				return;
			}
		}
	}
}

static void
render_chunks(struct chunks *cnks, struct opengl_ui_ctx *ctx)
{
	glMultiDrawElementsBaseVertex(
		GL_TRIANGLES,
		s_chunk.draw_counts,
		GL_UNSIGNED_SHORT,
		s_chunk.draw_indices,
		s_chunk.count,
		s_chunk.draw_baseverts);
}

static void
render_ents(struct hdarr *ents, struct hdarr *cnks, struct opengl_ui_ctx *ctx,
	struct simulation *sim)
{
	struct ent *emem = darr_raw_memory(hdarr_darr(ents));
	size_t i, j = 0, len = hdarr_len(ents);

	float positions[256 * 3] = { 0 };
	uint32_t types[256] = { 0 };
	size_t skipped = 0, rendered = 0;

	for (i = 0; i < len; ++i) {
		if (!point_in_rect(&emem[i].pos, &ctx->ref)) {
			++skipped;
			continue;
		}

		struct point p = nearest_chunk(&emem[i].pos);
		struct chunk *ck = hdarr_get(cnks, &p);

		positions[(j * 3) + 0] = emem[i].pos.x;
		positions[(j * 3) + 1] = emem[i].pos.y;
		if (ck) {
			p = point_sub(&emem[i].pos, &ck->pos);
			if (ck->tiles[p.x][p.y] <= tile_water) {
				positions[(j * 3) + 2] = -2.0;
			} else {
				positions[(j * 3) + 2] = 0.5 + ck->heights[p.x][p.y];
			}
		}

		if ((types[j] = emem[i].type) == et_worker) {
			if (emem[i].alignment == sim->assigned_motivator) {
				types[j] = et_elf_friend;
			} else {
				types[j] = et_elf_foe;
			}
		}

		if (++j >= 256) {
			glUniform1uiv(s_ent.types, j, types);
			glUniform3fv(s_ent.positions, j, positions);
			glDrawArraysInstanced(GL_TRIANGLES, 0, 36, j);
			j = 0;
		}

		++rendered;
	}

	glUniform1uiv(s_ent.types, j, types);
	glUniform3fv(s_ent.positions, j, positions);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, j);
}

static void
setup_hightlight_block(float time, vec4 clr, struct point *curs)
{
	float sel[8][3][3] = { 0 };
	uint8_t i;
	struct point np;
	chunk_mesh *ck;
	int x, y, ii;

	if (s_selection.count > MAX_RENDERED_HL) {
		return;
	}

	np = nearest_chunk(curs);

	if ((ck = hdarr_get(s_chunk.hd, &np))) {
		np = point_sub(curs, &np);
	}
	for (i = 0; i < 8; ++i) {
		x = np.x + i % 2;
		y = np.y + (i % 4) / 2;
		ii = y * MESH_DIM + x;

		sel[i][0][0] = curs->x + (i % 2) - 0.5;
		sel[i][0][1] = (2.5 * (1 - (i / 4))) + (ck ? (*ck)[ii].pos[1] : 0.0f);
		sel[i][0][2] = curs->y + ((i % 4) / 2) - 0.5;

		float br = (cos(time * 15) + 1) * 0.5;

		sel[i][1][0] = br * clr[0];
		sel[i][1][1] = br * clr[1];
		sel[i][1][2] = br * clr[2];

		sel[i][2][0] = 1;
		sel[i][2][1] = 1;
		sel[i][2][2] = 1;
	}

	glBufferSubData(GL_ARRAY_BUFFER,
		s_selection.count * sizeof(highlight_block),
		sizeof(highlight_block),
		sel);

	s_selection.draw_counts[s_selection.count] = 30;
	((GLvoid **)s_selection.draw_indices)[s_selection.count] = (void *)0;
	s_selection.draw_baseverts[s_selection.count] = s_selection.count * 8;

	++s_selection.count;
}

static void
setup_action_r(float time, int r, struct point *curs)
{
	struct point p, q;

	float i;
	const float points = r < 8 ? 8.0f : r;

	vec4 clr = { 1, 1, 1, 1 };

	for (i = 0; i < 2.0 * PI; i += (2.0 * PI / points)) {
		p.y = roundf(r * cos(i));
		p.x = roundf(r * sin(i));

		q = point_add(curs, &p);

		setup_hightlight_block(time, clr, &q);
	}
}

static void
setup_action_harvest(float time, struct chunks *chunks, struct point *curs,
	enum tile tgt, int r)
{
	int x, y;

	vec4 clr = { 1, 1, 1, 1 };

	for (x = -r; x < r * 2; ++x) {
		for (y = -r; y < r * 2; ++y) {
			if ((x * x) + (y * y) > r * r) {
				continue;
			}

			struct point p = { x, y };
			p = point_add(&p, curs);
			struct point q = nearest_chunk(&p);
			struct chunk *ck;
			if ((ck = hdarr_get(chunks->hd, &q))) {
				q = point_sub(&p, &q);

				if (tgt == ck->tiles[q.x][q.y]) {
					setup_hightlight_block(time, clr, &p);
				}
			}
		}
	}
}
static void
setup_action_build(float time, struct chunks *chunks, struct point *curs,
	enum building b)
{
	const struct blueprint *bp = &blueprints[b];
	struct point cp, rp;
	struct chunk *ck;
	size_t i;

	vec4 clr[2] = {
		{ 1, 1, 1, 1 },
		{ 1, 0, 0, 1 }
	};

	for (i = 0; i < BLUEPRINT_LEN; ++i) {
		if (!(bp->len & (1 << i))) {
			break;
		}

		rp = point_add(curs, &bp->blocks[i].p);
		cp = nearest_chunk(&rp);
		if ((ck = hdarr_get(chunks->hd, &cp))) {
			cp = point_sub(&rp, &ck->pos);

			if (gcfg.tiles[ck->tiles[cp.x][cp.y]].foundation) {
				setup_hightlight_block(time, clr[0], &rp);
				continue;
			}
		}


		setup_hightlight_block(time, clr[1], &rp);
	}
}

static void
setup_action_sel(float time, struct chunks *chunks, struct point *curs, const struct action *act)
{
	vec4 clr = { 1, 1, 1, 1 };

	switch (act->type) {
	case at_harvest:
		setup_action_harvest(time, chunks, curs, act->tgt, act->range.r);
		break;
	case at_build:
		setup_action_build(time, chunks, curs, act->tgt);
		break;
	case at_fight:
		setup_action_r(time, act->range.r, curs);
		break;
	case at_carry:
		setup_action_r(time, act->range.r, curs);
		break;
	default:
		setup_hightlight_block(time, clr, curs);
		break;
	}
}

static void
render_selection(struct hiface *hf, struct opengl_ui_ctx *ctx)
{
	float time = glfwGetTime();
	struct point curs = point_add(&hf->view, &hf->cursor);
	s_selection.count = 0;

	//setup_hightlight_block(time, &curs);
	setup_action_sel(time, hf->sim->w->chunks, &curs, &hf->next_act);

	glMultiDrawElementsBaseVertex(
		GL_TRIANGLES,
		s_selection.draw_counts,
		GL_UNSIGNED_BYTE,
		s_selection.draw_indices,
		s_selection.count,
		s_selection.draw_baseverts);
}

static void
fix_cursor(const struct rectangle *r, struct point *vu, struct point *c)
{
	if (c->x <= 0) {
		c->x = 0;
	} else if (c->x >= r->width) {
		c->x = r->width;
	}

	if (c->y <= 0) {
		c->y = 0;
	} else if (c->y >= r->height) {
		c->y = r->height;
	}
}

void
render_world(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	mat4 mview;
	float w, h;
	static struct rectangle oref = { 0 };
	bool ref_changed = false;

	if (cam.changed || ctx->resized || !points_equal(&hf->view, &ctx->ref.pos)) {
		ctx->ref.pos = hf->view;

		if (!cam.unlocked) {
			w = cam.pos[1] * (float)ctx->width / (float)ctx->height * 0.48;
			h = cam.pos[1] * tanf(FOV / 2) * 2;
			cam.pos[0] = ctx->ref.pos.x + w * 0.5;
			cam.pos[2] = ctx->ref.pos.y + h * 0.5;
			ctx->ref.width = w;
			ctx->ref.height = h;
		}

		cam.tgt[0] = cos(cam.yaw) * cos(cam.pitch);
		cam.tgt[1] = sin(cam.pitch);
		cam.tgt[2] = sin(cam.yaw) * cos(cam.pitch);

		gen_look_at(&cam, mview);
		cam.changed = true;

		if ((ref_changed = memcmp(&oref, &ctx->ref, sizeof(struct rectangle)))) {
			oref = ctx->ref;
		}
	}

	/* ents */

	glUseProgram(s_ent.id);
	glBindVertexArray(s_ent.vao);

	if (cam.changed) {
		glUniformMatrix4fv(s_ent.view, 1, GL_TRUE, (float *)mview);
		glUniform3fv(s_ent.view_pos, 1, cam.pos);
	}

	render_ents(hf->sim->w->ents, hf->sim->w->chunks->hd, ctx, hf->sim);

	/* chunks */

	glUseProgram(s_chunk.id);
	glBindVertexArray(s_chunk.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_chunk.ebo);
	glBindBuffer(GL_ARRAY_BUFFER, s_chunk.vbo);

	if (cam.changed) {
		glUniformMatrix4fv(s_chunk.view, 1, GL_TRUE, (float *)mview);
		glUniform3fv(s_chunk.view_pos, 1, cam.pos);
	}

	if (ref_changed || hf->sim->changed.chunks) {
		/* orphan previous buffer */
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(chunk_mesh) * MAX_RENDERED_CHUNKS,
			NULL, GL_DYNAMIC_DRAW);

		hdarr_clear(s_chunk.hd);
		setup_chunks(hf->sim->w->chunks, ctx);
	}

	render_chunks(hf->sim->w->chunks, ctx);

	/* selection */

	glUseProgram(s_selection.id);
	glBindVertexArray(s_selection.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_selection.ebo);
	glBindBuffer(GL_ARRAY_BUFFER, s_selection.vbo);

	if (cam.changed) {
		glUniformMatrix4fv(s_selection.view, 1, GL_TRUE, (float *)mview);
		glUniform3fv(s_selection.view_pos, 1, cam.pos);

		/* last usage of cam.changed */
		cam.changed = false;
	}


	if (hf->im != im_select) {
		return;
	}

	fix_cursor(&ctx->ref, &hf->view, &hf->cursor);

	render_selection(hf, ctx);
}
