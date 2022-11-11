#include "posix.h"

#include <assert.h>

#include "shared/math/linalg.h"
#include "shared/types/darr.h"
#include "shared/ui/gl/render/shapes.h"
#include "shared/ui/gl/shader.h"
#include "shared/ui/gl/window.h"

enum {
	su_proj = UNIFORM_START_RP_FINAL,
};

enum shape_type {
	shape_type_rect,
	shape_type_tri,
};

struct shape {
	enum shape_type type;
	union {
		struct {
			float x, y, h, w;
		} rect;
		struct {
			float x1, y1, x2, y2, x3, y3;
		} tri;
	} s;
	vec4 clr;
};

struct shape_vert {
	float x, y;
	vec4 clr;
};

static struct {
	struct shader shader;
	struct darr shapes, verts;
} state;

bool
render_shapes_setup(void)
{
	struct shader_spec spec = {
		.src = {
			[rp_final] = {
				{ "shapes.vert", GL_VERTEX_SHADER   },
				{ "shapes.frag", GL_FRAGMENT_SHADER },
			}
		},
		.uniform = {
			[rp_final] = {
				{ su_proj, "proj" },
			},
		},
		.attribute = {
			{
				{ 2, GL_FLOAT, bt_vbo,  0, 0 },
				{ 4, GL_FLOAT, bt_vbo,  0, 0 },
			}
		},
		.uniform_blacklist = { [rp_final] = 0xffff },
	};

	if (!shader_create(&spec, &state.shader)) {
		return false;
	}

	darr_init(&state.shapes, sizeof(struct shape));
	darr_init(&state.verts, sizeof(struct shape_vert));

	return true;
}

static void
push_vert(float x, float y, vec4 clr)
{
	darr_push(&state.verts, &(struct shape_vert){
		x, y, .clr = { clr[0], clr[1], clr[2], clr[3] }
	});
}

uint32_t
render_shapes_add_tri(float x1, float y1, float x2, float y2, float x3, float y3, vec4 clr)
{
	return darr_push(&state.shapes, &(struct shape) {
		.type = shape_type_tri,
		.s.tri = { x1, y1, x2, y2, x3, y3 },
		.clr = { clr[0], clr[1], clr[2], clr[3] }
	});
}

uint32_t
render_shapes_add_rect(float x, float y, float h, float w, vec4 clr)
{
	return darr_push(&state.shapes, &(struct shape) {
		.type = shape_type_rect,
		.s.rect = { .x = x, .y = y, .h = h, .w = w, },
		.clr = { clr[0], clr[1], clr[2], clr[3] }
	});
}

void
render_shapes_resize(uint32_t i, float h, float w)
{
	struct shape *s = darr_get(&state.shapes, i);

	assert(s->type == shape_type_rect);

	s->s.rect.h = h;
	s->s.rect.w = w;
}

void
render_shapes_clear(void)
{
	darr_clear(&state.shapes);
	darr_clear(&state.verts);
}

void
render_shapes_update_proj(mat4 proj)
{
	glUseProgram(state.shader.id[rp_final]);
	glUniformMatrix4fv(state.shader.uniform[rp_final][su_proj], 1,
		GL_TRUE, (float *)proj);
}

void
render_shapes(void)
{
	glUseProgram(state.shader.id[rp_final]);
	glBindVertexArray(state.shader.vao[rp_final][0]);

	int32_t i;
	struct shape *s;
	for (i = darr_len(&state.shapes) - 1; i >= 0; --i) {
		s = darr_get(&state.shapes, i);

		switch (s->type) {
		case shape_type_rect:
			push_vert(s->s.rect.x,               s->s.rect.y,               s->clr);
			push_vert(s->s.rect.x + s->s.rect.w, s->s.rect.y,               s->clr);
			push_vert(s->s.rect.x,               s->s.rect.y + s->s.rect.h, s->clr);
			push_vert(s->s.rect.x + s->s.rect.w, s->s.rect.y,               s->clr);
			push_vert(s->s.rect.x + s->s.rect.w, s->s.rect.y + s->s.rect.h, s->clr);
			push_vert(s->s.rect.x,               s->s.rect.y + s->s.rect.h, s->clr);
			break;
		case shape_type_tri:
			push_vert(s->s.tri.x1, s->s.tri.y1, s->clr);
			push_vert(s->s.tri.x2, s->s.tri.y2, s->clr);
			push_vert(s->s.tri.x3, s->s.tri.y3, s->clr);
			break;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, state.shader.buffer[bt_vbo]);
	glBufferData(GL_ARRAY_BUFFER, darr_size(&state.verts),
		darr_raw_memory(&state.verts), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, darr_len(&state.verts));
}
