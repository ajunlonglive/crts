#include "posix.h"

#include "shared/math/linalg.h"
#include "shared/opengl/render/shapes.h"
#include "shared/opengl/shader.h"
#include "shared/opengl/window.h"
#include "shared/types/darr.h"

enum {
	su_proj = UNIFORM_START_RP_FINAL,
};

enum shape_type {
	shape_type_rect,
};

struct shape {
	enum shape_type type;
	float x, y, h, w;
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
render_shapes_add_rect(float x, float y, float h, float w, vec4 clr)
{
	return darr_push(&state.shapes, &(struct shape) {
		.type = shape_type_rect,
		.x = x, .y = y, .h = h, .w = w,
		.clr = { clr[0], clr[1], clr[2], clr[3] }
	});
}

void
render_shapes_resize(uint32_t i, float h, float w)
{
	struct shape *s = darr_get(&state.shapes, i);

	s->h = h;
	s->w = w;
}

void
render_shapes_clear(void)
{
	darr_clear(&state.shapes);
	darr_clear(&state.verts);
}

void
render_shapes(struct gl_win *win, mat4 proj)
{
	glUseProgram(state.shader.id[rp_final]);
	glBindVertexArray(state.shader.vao[rp_final][0]);

	if (win->resized) {
		glUniformMatrix4fv(state.shader.uniform[rp_final][su_proj], 1,
			GL_TRUE, (float *)proj);
	}

	int32_t i;
	struct shape *s;
	for (i = darr_len(&state.shapes) - 1; i >= 0; --i) {
		s = darr_get(&state.shapes, i);

		switch (s->type) {
		case shape_type_rect:
			push_vert(s->x,        s->y,        s->clr);
			push_vert(s->x + s->w, s->y,        s->clr);
			push_vert(s->x,        s->y + s->h, s->clr);
			push_vert(s->x + s->w, s->y,        s->clr);
			push_vert(s->x + s->w, s->y + s->h, s->clr);
			push_vert(s->x,        s->y + s->h, s->clr);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, state.shader.buffer[bt_vbo]);
	glBufferData(GL_ARRAY_BUFFER, darr_size(&state.verts),
		darr_raw_memory(&state.verts), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, darr_len(&state.verts));
}
