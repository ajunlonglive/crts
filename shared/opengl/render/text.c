#include "posix.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef CRTS_COMPTIME
#include "font_atlas.h"
#else
float font_atlas[256][2] = { 0 };
float font_atlas_cdim[2] = { 0 };
#endif

#define BUFLEN 256

#include "shared/math/linalg.h"
#include "shared/opengl/render/text.h"
#include "shared/opengl/shader.h"
#include "shared/opengl/util.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

struct charspec {
	float pos[2]; /* in_vertex_off */
	float tex[2]; /* in_tex_coord_off */
	vec4 color;
};

enum {
	su_font_atlas = UNIFORM_START_RP_FINAL,
	su_proj,
};

static struct {
	uint32_t height, width;
	struct shader shader;
	struct { int32_t font_atlas; } textures;
	struct darr charspecs;
} text_state;

bool
render_text_setup(void)
{
	uint32_t indices[] = { 0, 1, 2, 2, 1, 3 };
	float text_quad[] = { /* pos x, pos y, tex coord x, tex coord y */
		0.0f, 1.0f, 0.0f,               font_atlas_cdim[1],
		1.0f, 1.0f, font_atlas_cdim[0], font_atlas_cdim[1],
		0.0f, 0.0f, 0.0f,               0.0f,
		1.0f, 0.0f, font_atlas_cdim[0], 0.0f,
	};

	struct shader_spec spec = {
		.src = {
			[rp_final] = {
				{ "text.vert", GL_VERTEX_SHADER   },
				{ "text.frag", GL_FRAGMENT_SHADER },
			}
		},
		.uniform = {
			[rp_final] = {
				{ su_font_atlas, "font_atlas" },
				{ su_proj, "proj" },
			},
		},
		.attribute = {
			{
				{ 2, GL_FLOAT, bt_vbo,  0, 0 },
				{ 2, GL_FLOAT, bt_vbo,  0, 0 },
				{ 2, GL_FLOAT, bt_nvbo, 0, 1 },
				{ 2, GL_FLOAT, bt_nvbo, 0, 1 },
				{ 4, GL_FLOAT, bt_nvbo, 0, 1 }
			}
		},
		.static_data = {
			{ indices, sizeof(uint32_t) * 6, bt_ebo },
			{ text_quad, sizeof(float) * 16, bt_vbo }
		},
		.uniform_blacklist = { [rp_final] = 0xffff },
	};

	if (!shader_create(&spec, &text_state.shader)) {
		return false;
	} else if ((text_state.textures.font_atlas = load_tex("font_atlas.tga",
		GL_CLAMP_TO_BORDER, GL_NEAREST)) == -1) {
		return false;
	}

	darr_init(&text_state.charspecs, sizeof(struct charspec));

	glUseProgram(text_state.shader.id[rp_final]);
	glUniform1i(text_state.shader.uniform[rp_final][su_font_atlas], 0);

	return true;
}

void
render_text_add(float *x, float *y, const vec4 clr, const char *str)
{
	for (; *str; ++str, ++(*x)) {
		darr_push(&text_state.charspecs, &(struct charspec) {
			.pos = { *x, *y },
			.tex = { font_atlas[(uint8_t)*str][0], font_atlas[(uint8_t)*str][1] },
			.color = { clr[0], clr[1], clr[2], clr[3] }
		});
	}
}

void
render_text_commit(void)
{
	glBindBuffer(GL_ARRAY_BUFFER, text_state.shader.buffer[bt_nvbo]);
	glBufferData(GL_ARRAY_BUFFER, darr_size(&text_state.charspecs),
		darr_raw_memory(&text_state.charspecs), GL_DYNAMIC_DRAW);
}

void
render_text_clear(void)
{
	darr_clear(&text_state.charspecs);
}

void
render_text(struct gl_win *win, mat4 proj)
{
	glUseProgram(text_state.shader.id[rp_final]);
	glBindVertexArray(text_state.shader.vao[rp_final][0]);

	if (win->resized) {
		glUniformMatrix4fv(text_state.shader.uniform[rp_final][su_proj], 1,
			GL_TRUE, (float *)proj);

		text_state.height = win->height;
		text_state.width = win->width;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, text_state.textures.font_atlas);

	glDrawElementsInstanced(GL_TRIANGLES,
		6,
		GL_UNSIGNED_INT,
		(void *)(0),
		darr_len(&text_state.charspecs)
		);
}
