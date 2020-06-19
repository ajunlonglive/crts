#include "posix.h"

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef INCLUDE_FONT_ATLAS
#include "font_atlas.h"
#else
#define FONT_ATLAS_WIDTH 0
#define FONT_ATLAS_HEIGHT 0
#define FONT_ATLAS_CWIDTH 0
#define FONT_ATLAS_CHEIGHT 0
float font_atlas[256][2] = { 0 };
float font_atlas_cdim[2] = { 0 };
#endif

#define CHARSCALE 16.0f
#define BUFLEN 256

#include "client/ui/opengl/loaders/shader.h"
#include "client/ui/opengl/loaders/tga.h"
#include "client/ui/opengl/render/text.h"
#include "client/ui/opengl/ui.h"
#include "shared/math/linalg.h"
#include "shared/util/log.h"

float quad_verts[] = {
	// first triangle
	0.5f,  0.5f, 1.0f, 1.0f,  // top right
	0.5f, -0.5f, 1.0f, 0.0f,  // bottom right
	-0.5f,  0.5f, 0.0f, 1.0f, // top left

	// second triangle
	0.5f, -0.5f,  1.0f, 0.0f,  // bottom right
	-0.5f, -0.5f, 0.0f, 0.0f, // bottom left
	-0.5f,  0.5f, 0.0f, 1.0f  // top left
};

static struct {
	uint32_t texture_id;
	uint32_t vao, vbo;
	uint32_t pid;
	struct {
		uint32_t atlasCoords, string, charDims, proj, iniPos, uclr,
			 scale;
	} uni;
	uint32_t height, width;
} text_state;

bool
render_text_setup(void)
{
	void *data;

	struct shader_src src[] = {
		{ "text.vert", GL_VERTEX_SHADER   },
		{ "text.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!link_shaders(src, &text_state.pid)) {
		return false;
	}

	glUseProgram(text_state.pid);

	text_state.uni.atlasCoords = glGetUniformLocation(text_state.pid, "atlasCoords");
	text_state.uni.string      = glGetUniformLocation(text_state.pid, "string");
	text_state.uni.charDims    = glGetUniformLocation(text_state.pid, "charDims");
	text_state.uni.proj        = glGetUniformLocation(text_state.pid, "proj");
	text_state.uni.iniPos      = glGetUniformLocation(text_state.pid, "iniPos");
	text_state.uni.uclr        = glGetUniformLocation(text_state.pid, "uclr");
	text_state.uni.scale       = glGetUniformLocation(text_state.pid, "scale");

	glGenTextures(1, &text_state.texture_id);
	glBindTexture(GL_TEXTURE_2D, text_state.texture_id);

	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// load and generate the texture
	if ((data = load_tga("font_atlas.tga"))) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FONT_ATLAS_WIDTH,
			FONT_ATLAS_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}else {
		LOG_W("failed to initialize font atlas texture");
		return false;
	}

	glUniform2fv(text_state.uni.atlasCoords, 256, (float *)font_atlas);
	glUniform2fv(text_state.uni.charDims, 1, (float *)font_atlas_cdim);

	glGenVertexArrays(1, &text_state.vao);
	glGenBuffers(1, &text_state.vbo);

	glBindVertexArray(text_state.vao);

	glBindBuffer(GL_ARRAY_BUFFER, text_state.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, quad_verts, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	// texture attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
		(void *)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	return true;
}

void
text_setup_render(struct opengl_ui_ctx *ctx)
{
	glUseProgram(text_state.pid);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, text_state.texture_id);
	glBindVertexArray(text_state.vao);

	if (ctx->resized) {
		mat4 ortho, mscale, proj;
		vec4 scale = { CHARSCALE, CHARSCALE, 0.0, 0.0 };

		gen_ortho_mat4(0.0, (float)ctx->width, 0.0, (float)ctx->height, ortho);
		gen_scale_mat4(scale, mscale);

		/* TODO: we could just use vec4_mat_mat4 here and avoid generating
		 * mscale */
		mat4_mult_mat4(ortho, mscale, proj);

		glUniformMatrix4fv(text_state.uni.proj, 1, GL_TRUE, (float *)proj);

		text_state.height = ctx->height;
		text_state.width = ctx->width;
	}
}

void
screen_coords_to_text_coords(float x, float y, float *sx, float *sy)
{
	*sx = x / CHARSCALE;
	*sy = (text_state.height - y) / CHARSCALE;
}

size_t
gl_write_string_centered(float x, float y, float scale, vec4 clr,
	const char *str)
{
	size_t len = strlen(str);
	float hx = len * scale / 2, hy = scale / 2;

	return gl_write_string(x - hx, y - hy, scale, clr, str);
}

size_t
gl_write_string(float x, float y, float scale, vec4 clr, const char *str)
{
	uint32_t bbuf[BUFLEN] = { 0 };
	const char *p;
	size_t l = 0;
	float iniPos[] = { x / scale, y / scale };

	for (p = str; *p != '\0'; ++p, ++l) {
		bbuf[l] = *p;
	}

	glUniform4fv(text_state.uni.uclr, 1, clr);
	glUniform1uiv(text_state.uni.string, l, bbuf);
	glUniform2fv(text_state.uni.iniPos, 1, iniPos);
	glUniform1fv(text_state.uni.scale, 1, &scale);

	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, l);

	return l;
}

size_t
gl_printf(float x, float y, const char *fmt, ...)
{
	char buf[BUFLEN] = { 0 };
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, 255, fmt, ap);
	va_end(ap);

	vec4 clr = { 1.0, 1.0, 1.0, 0.6 };

	x = (x < 0 ? (text_state.width / CHARSCALE) + x : x) + 0.5;
	y = (y < 0 ? (text_state.height / CHARSCALE) + y : y) + 0.5;

	return gl_write_string(x, y, 1.0, clr, buf);
}
