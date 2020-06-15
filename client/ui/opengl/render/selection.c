#include "posix.h"

#include <math.h>

#include "client/ui/opengl/render/chunks.h"
#include "shared/constants/blueprints.h"
#include "shared/constants/globals.h"
#include "client/ui/opengl/render/selection.h"
#include "client/ui/opengl/winutil.h"
#include "client/ui/opengl/globals.h"

static struct hdarr *chunk_meshes;

#define MAX_RENDERED_HL 2048
typedef float highlight_block[8][3][3];

static struct {
	uint32_t id;
	uint32_t vao, vbo, ebo;
	uint32_t view, proj, view_pos, pulse;
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
	s_selection.pulse     = glGetUniformLocation(s_selection.id, "pulse");

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
		sizeof(highlight_block) * MAX_RENDERED_HL,
		NULL, GL_DYNAMIC_DRAW);

	return true;
}

static void
setup_hightlight_block(float h, vec4 clr, struct point *curs)
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

	if ((ck = hdarr_get(chunk_meshes, &np))) {
		np = point_sub(curs, &np);
	}

	for (i = 0; i < 8; ++i) {
		x = np.x + i % 2;
		y = np.y + (i % 4) / 2;
		ii = y * MESH_DIM + x;

		sel[i][0][0] = curs->x + (i % 2) - 0.5;
		sel[i][0][1] = (h * (1 - (i / 4))) + (ck ? (*ck)[ii].pos[1] : 0.0f);
		sel[i][0][2] = curs->y + ((i % 4) / 2) - 0.5;

		sel[i][1][0] = clr[0];
		sel[i][1][1] = clr[1];
		sel[i][1][2] = clr[2];

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
setup_action_r(int r, struct point *curs)
{
	struct point p, q;

	float i;
	const float points = r < 8 ? 8.0f : r;

	vec4 clr = { 1, 1, 1, 1 };

	for (i = 0; i < 2.0 * PI; i += (2.0 * PI / points)) {
		p.y = roundf(r * cos(i));
		p.x = roundf(r * sin(i));

		q = point_add(curs, &p);

		setup_hightlight_block(0.1, clr, &q);
	}
}

static void
setup_action_harvest(struct chunks *chunks, struct point *curs,
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
					setup_hightlight_block(0.1, clr, &p);
				}
			}
		}
	}
}
static void
setup_action_build(struct chunks *chunks, struct point *curs, enum building b)
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
				setup_hightlight_block(1.0, clr[0], &rp);
				continue;
			}
		}


		setup_hightlight_block(1.0, clr[1], &rp);
	}
}

static void
setup_action_sel(struct chunks *chunks, struct point *curs, const struct action *act)
{
	vec4 clr = { 0, 1, 1, 1 };

	switch (act->type) {
	case at_harvest:
		setup_action_harvest(chunks, curs, act->tgt, act->range.r);

		setup_hightlight_block(1.0, clr, curs);
		break;
	case at_build:
		setup_action_build(chunks, curs, act->tgt);
		break;
	case at_fight:
		setup_action_r(act->range.r, curs);

		setup_hightlight_block(1.0, clr, curs);
		break;
	case at_carry:
		setup_action_r(act->range.r, curs);

		setup_hightlight_block(1.0, clr, curs);
		break;
	default:
		setup_hightlight_block(1.0, clr, curs);
		break;
	}
}

static void
render_selection_setup(struct hiface *hf, struct opengl_ui_ctx *ctx)
{
	struct point curs = point_add(&hf->view, &hf->cursor);
	s_selection.count = 0;

	size_t i;
	for (i = 0; i < ACTION_HISTORY_SIZE; ++i) {
		if (hf->sim->action_history[i].type) {
			setup_action_sel(hf->sim->w->chunks,
				&hf->sim->action_history[i].range.center,
				&hf->sim->action_history[i]);
		}
	}

	setup_action_sel(hf->sim->w->chunks, &curs, &hf->next_act);
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
render_selection(struct hiface *hf, struct opengl_ui_ctx *ctx,
	struct hdarr *cms, mat4 mview, bool reset_chunks)
{
	static struct point oc, ov;

	chunk_meshes = cms;

	glUseProgram(s_selection.id);
	glBindVertexArray(s_selection.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_selection.ebo);
	glBindBuffer(GL_ARRAY_BUFFER, s_selection.vbo);

	if (cam.changed) {
		glUniformMatrix4fv(s_selection.proj, 1, GL_TRUE, (float *)ctx->mproj);
		glUniformMatrix4fv(s_selection.view, 1, GL_TRUE, (float *)mview);
		glUniform3fv(s_selection.view_pos, 1, cam.pos);
	}

	fix_cursor(&ctx->ref, &hf->view, &hf->cursor);

	if (reset_chunks
	    || hf->sim->changed.actions
	    || !points_equal(&oc, &hf->cursor)
	    || !points_equal(&ov, &hf->view)
	    || hf->next_act_changed) {
		/* orphan previous buffer */
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(highlight_block) * MAX_RENDERED_HL,
			NULL, GL_DYNAMIC_DRAW);

		render_selection_setup(hf, ctx);
	}

	if (hf->im == im_select) {
		glUniform1fv(s_selection.pulse, 1, &ctx->pulse);

		glMultiDrawElementsBaseVertex(
			GL_TRIANGLES,
			s_selection.draw_counts,
			GL_UNSIGNED_BYTE,
			s_selection.draw_indices,
			s_selection.count,
			s_selection.draw_baseverts);
	}

	oc = hf->cursor;
	ov = hf->view;
}
