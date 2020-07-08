#include "posix.h"

#include <assert.h>

#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/loaders/shader.h"
#include "client/ui/opengl/render/water.h"
#include "shared/util/log.h"

static uint32_t
attach_color(uint32_t w, uint32_t h)
{
	uint32_t tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, tex, 0);

	return tex;
}

static uint32_t
attach_db(uint32_t w, uint32_t h)
{
	uint32_t db;
	glGenRenderbuffers(1, &db);

	glBindRenderbuffer(GL_RENDERBUFFER, db);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, db);

	return db;
}

static uint32_t
attach_dtex(uint32_t w, uint32_t h)
{
	uint32_t dtex;

	glGenTextures(1, &dtex);
	glBindTexture(GL_TEXTURE_2D, dtex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, dtex, 0);

	return dtex;
}

static struct {
	uint32_t id;
	uint32_t vao, vbo, ebo;
	uint32_t viewproj;
	int32_t reflect_tex, refract_tex, depth_tex;
} water_shader;

bool
render_world_setup_water(struct water_fx *wfx)
{
	/* refract */
	glGenFramebuffers(1, &wfx->refract_fb);
	glBindFramebuffer(GL_FRAMEBUFFER, wfx->refract_fb);
	wfx->refract_tex = attach_color(wfx->refract_w, wfx->refract_h);
	wfx->refract_dtex = attach_dtex(wfx->refract_w, wfx->refract_h);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/* reflect */
	glGenFramebuffers(1, &wfx->reflect_fb);
	glBindFramebuffer(GL_FRAMEBUFFER, wfx->reflect_fb);
	wfx->reflect_tex = attach_color(wfx->reflect_w, wfx->reflect_h);
	wfx->reflect_db = attach_db(wfx->reflect_w, wfx->reflect_h);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	struct shader_src src[] = {
		{ "water.vert", GL_VERTEX_SHADER },
		{ "water.frag", GL_FRAGMENT_SHADER },
	};

	if (!link_shaders(src, &water_shader.id)) {
		return false;
	}

	glGenBuffers(1, &water_shader.vbo);
	glGenBuffers(1, &water_shader.ebo);

	glGenVertexArrays(1, &water_shader.vao);
	glBindVertexArray(water_shader.vao);
	glBindBuffer(GL_ARRAY_BUFFER, water_shader.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, water_shader.ebo);

	uint32_t indices[] = { 0, 1, 2, 1, 2, 3 };
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6, indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void *)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	water_shader.viewproj = glGetUniformLocation(water_shader.id, "viewproj");
	water_shader.reflect_tex = glGetUniformLocation(water_shader.id, "reflect_tex");
	water_shader.refract_tex = glGetUniformLocation(water_shader.id, "refract_tex");
	water_shader.depth_tex = glGetUniformLocation(water_shader.id, "depth_tex");

	glUseProgram(water_shader.id);
	glUniform1i(water_shader.reflect_tex, 0);
	glUniform1i(water_shader.refract_tex, 1);
	glUniform1i(water_shader.depth_tex, 2);

	return true;
}

void
render_water_setup_frame(struct opengl_ui_ctx *ctx)
{
	if (!ctx->ref_changed) {
		return;
	}

	float quad[] = {
		ctx->ref.pos.x, 0.0, ctx->ref.pos.y,

		ctx->ref.pos.x + ctx->ref.width, 0.0, ctx->ref.pos.y,

		ctx->ref.pos.x, 0.0, ctx->ref.pos.y + ctx->ref.height,

		ctx->ref.pos.x + ctx->ref.width, 0.0, ctx->ref.pos.y + ctx->ref.height,
	};

	glBindBuffer(GL_ARRAY_BUFFER, water_shader.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3  * 4, quad, GL_DYNAMIC_DRAW);
}

void
render_water(struct opengl_ui_ctx *ctx)
{
	glUseProgram(water_shader.id);

	glUniformMatrix4fv(water_shader.viewproj, 1, GL_TRUE, (float *)cam.proj);

	glBindVertexArray(water_shader.vao);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *)0);
}
