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
	float pos[2];
	float tex[2];
	vec4 color;
};

static struct { uint32_t height, width; float scale; } text_state;

struct shader text_shader;
enum {
	su_font_atlas = UNIFORM_START_RP_FINAL,
	su_proj,
};

struct { int32_t font_atlas; } textures;

struct darr charspecs = { 0 };

bool
render_text_setup(float scale)
{
	uint32_t indices[] = { 0, 1, 2, 2, 1, 3 };
	float text_quad[] = {
		0.0f, 1.0f, 0.0, font_atlas_cdim[1],
		1.0f, 1.0f, font_atlas_cdim[0], font_atlas_cdim[1],
		0.0f, 0.0f, 0.0, 0.0,
		1.0f, 0.0f, font_atlas_cdim[0], 0.0,
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

	if (!shader_create(&spec, &text_shader)) {
		return false;
	} else if ((textures.font_atlas = load_tex("font_atlas.tga",
		GL_CLAMP_TO_BORDER, GL_NEAREST)) == -1) {
		return false;
	}

	darr_init(&charspecs, sizeof(struct charspec));

	glUseProgram(text_shader.id[rp_final]);
	glUniform1i(text_shader.uniform[rp_final][su_font_atlas], 0);

	text_state.scale = scale;

	return true;
}

void
screen_coords_to_text_coords(float x, float y, float *sx, float *sy)
{
	*sx = x < 0 ? (text_state.width  / text_state.scale) + x : x;
	*sy = y < 0 ? (text_state.height / text_state.scale) + y : y;
}

void
gl_write_char(float x, float y, const vec4 clr, char c)
{
	struct charspec spec = {
		.pos = { x, y },
		.tex = {
			font_atlas[(uint8_t)c][0],
			font_atlas[(uint8_t)c][1]
		},
		.color = { clr[0], clr[1], clr[2], clr[3] }
	};

	darr_push(&charspecs, &spec);
}

size_t
gl_write_string(float x, float y, float _scale, const vec4 clr, const char *str)
{
	size_t i = 0, len = strlen(str);

	for (i = 0; i < len; ++i) {
		gl_write_char(x + i, y, clr, str[i]);
	}

	return len;
}

size_t
gl_write_string_centered(float x, float y, float scale, const vec4 clr,
	const char *str)
{
	size_t len = strlen(str);
	float hx = len * scale / 2, hy = scale / 2;

	return gl_write_string(x - hx, y - hy, scale, clr, str);
}

bool
gl_mprintf(float sx, float sy, enum text_anchor anch, const struct pointf *c,
	void *ctx, interactive_text_cb cb, const char *fmt, ...)
{
	char buf[BUFLEN] = { 0 };
	size_t len;
	va_list ap;

	va_start(ap, fmt);
	len = vsnprintf(buf, BUFLEN - 1, fmt, ap);
	va_end(ap);

	float x, y;
	screen_coords_to_text_coords(sx, sy, &x, &y);

	if (anch == ta_right) {
		x -= (len - 1);
	}

	bool hover = c->x > x && c->x < x + len &&
		     c->y > y && c->y < y + 1;

	return cb(ctx, hover, x, y, buf);
}

size_t
gl_printf(float x, float y, enum text_anchor anch, const char *fmt, ...)
{
	char buf[BUFLEN] = { 0 };
	size_t len;
	va_list ap;

	va_start(ap, fmt);
	len = vsnprintf(buf, BUFLEN - 1, fmt, ap);
	va_end(ap);

	vec4 clr = { 1.0, 1.0, 1.0, 0.6 };

	if (anch == ta_right) {
		x -= len - 1;
	}

	return gl_write_string(x, y, 1.0, clr, buf);
}

static void
regen_proj_matrix(struct gl_win *win)
{
	mat4 ortho, mscale, proj;

	vec4 scale = { text_state.scale, text_state.scale, 0.0, 0.0 };
	gen_scale_mat4(scale, mscale);

	gen_ortho_mat4_from_lrbt(0.0, (float)win->width, 0.0, (float)win->height, ortho);

	mat4_mult_mat4(ortho, mscale, proj);

	glUniformMatrix4fv(text_shader.uniform[rp_final][su_proj], 1,
		GL_TRUE, (float *)proj);

	text_state.height = win->height;
	text_state.width = win->width;
}

void
render_text_commit(void)
{
	glBindBuffer(GL_ARRAY_BUFFER, text_shader.buffer[bt_nvbo]);
	glBufferData(GL_ARRAY_BUFFER, darr_size(&charspecs),
		darr_raw_memory(&charspecs), GL_DYNAMIC_DRAW);
}

void
render_text_clear(void)
{
	darr_clear(&charspecs);
}

void
render_text(struct gl_win *win)
{
	glUseProgram(text_shader.id[rp_final]);
	glBindVertexArray(text_shader.vao[rp_final][0]);

	if (win->resized) {
		regen_proj_matrix(win);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures.font_atlas);

	glDrawElementsInstanced(GL_TRIANGLES,
		6,
		GL_UNSIGNED_INT,
		(void *)(0),
		darr_len(&charspecs)
		);
}
