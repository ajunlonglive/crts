#include "posix.h"

#include "shared/math/linalg.h"
#include "shared/opengl/render/shapes.h"
#include "shared/opengl/shader.h"
#include "shared/opengl/window.h"
#include "shared/types/darr.h"

enum {
	su_proj = UNIFORM_START_RP_FINAL,
};

struct shape_vert {
	float x, y;
	vec4 clr;
};

static struct {
	struct shader shader;
	struct darr verts, verts_reversed;
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

	darr_init(&state.verts, sizeof(struct shape_vert));
	darr_init(&state.verts_reversed, sizeof(struct shape_vert));

	return true;
}

static void
render_shapes_add(float x, float y, vec4 clr)
{
	darr_push(&state.verts, &(struct shape_vert){
		x, y, .clr = { clr[0], clr[1], clr[2], clr[3] }
	});
}

void
render_shapes_add_rect(float x, float y, float h, float w, vec4 clr)
{
	render_shapes_add(x,     y,     clr);
	render_shapes_add(x + w, y,     clr);
	render_shapes_add(x,     y + h, clr);
	render_shapes_add(x + w, y,     clr);
	render_shapes_add(x + w, y + h, clr);
	render_shapes_add(x,     y + h, clr);
}

void
render_shapes_clear(void)
{
	darr_clear(&state.verts);
	darr_clear(&state.verts_reversed);
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
	for (i = darr_len(&state.verts) - 1; i >= 0; --i) {
		darr_push(&state.verts_reversed, darr_get(&state.verts, i));
	}

	glBindBuffer(GL_ARRAY_BUFFER, state.shader.buffer[bt_vbo]);
	glBufferData(GL_ARRAY_BUFFER, darr_size(&state.verts_reversed),
		darr_raw_memory(&state.verts_reversed), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, darr_len(&state.verts_reversed));
}
