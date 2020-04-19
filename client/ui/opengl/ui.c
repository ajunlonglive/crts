#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "client/ui/opengl/color_cfg.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/input.h"
#include "client/ui/opengl/solids.h"
#include "client/ui/opengl/ui.h"
#include "client/ui/opengl/winutil.h"
#include "shared/math/linalg.h"
#include "shared/sim/ent.h"
#include "shared/util/log.h"

struct opengl_ui_ctx {
	GLFWwindow* window;
	uint32_t prog_id, vao, vbo;
	struct {
		uint32_t mod, view, proj, clr;
	} uni;
};

static struct opengl_ui_ctx *global_ctx;

float tile_heights[tile_count] = {
	[tile_mountain] = 1,
	[tile_peak] = 2,
	[tile_water] = -1,
	[tile_deep_water] = -2,
};

static void
resize_callback(struct GLFWwindow *win, int width, int height)
{
	struct mat4 mproj;
	glViewport(0, 0, width, height);

	gen_perspective_mat4(0.47, (float)width / (float)height, 0.1, 1000.0, &mproj);
	glUniformMatrix4fv(global_ctx->uni.proj, 1, GL_TRUE, (float *)mproj.v);
}

struct opengl_ui_ctx *
opengl_ui_init(char *graphics_path)
{
	struct opengl_ui_ctx *ctx = calloc(1, sizeof(struct opengl_ui_ctx));
	global_ctx = ctx;

	struct shader_src prog1[] = {
		{ "client/ui/opengl/shaders/vertex.glsl",   GL_VERTEX_SHADER   },
		{ "client/ui/opengl/shaders/fragment.glsl", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!(ctx->window = init_window())) {
		goto free_exit;
	} else if (!link_shaders(prog1, &ctx->prog_id)) {
		goto free_exit;
	}

	if (!color_cfg(graphics_path)) {
		goto free_exit;
	}

	/* Set callbacks */
	set_input_callbacks(ctx->window);
	glfwSetFramebufferSizeCallback(ctx->window, resize_callback);

	glfwSetInputMode(ctx->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	/* wireframe mode */
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);

	ctx->uni.mod  = glGetUniformLocation(ctx->prog_id, "model");
	ctx->uni.view = glGetUniformLocation(ctx->prog_id, "view");
	ctx->uni.proj = glGetUniformLocation(ctx->prog_id, "proj");
	ctx->uni.clr  = glGetUniformLocation(ctx->prog_id, "color");

	glGenVertexArrays(1, &ctx->vao);
	glGenBuffers(1, &ctx->vbo);

	glBindVertexArray(ctx->vao);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * solid_cube.len,
		solid_cube.verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	/* TODO: right now we only have one of each of these */
	glUseProgram(ctx->prog_id);
	glBindVertexArray(ctx->vao);

	return ctx;

free_exit:
	free(ctx);
	return NULL;
}

struct render_ctx {
	struct vec4 pos;
	struct mat4 trans;
	struct opengl_ui_ctx *gl;
};

static enum iteration_result
render_chunk(void *_ctx, void *_ck)
{
	struct render_ctx *ctx = _ctx;
	struct chunk *ck = _ck;

	uint16_t i, j;

	for (i = 0; i < CHUNK_SIZE; ++i) {
		for (j = 0; j < CHUNK_SIZE; ++j) {
			ctx->pos.x = ck->pos.x + i;
			ctx->pos.y = tile_heights[ck->tiles[i][j]] - 1;
			ctx->pos.z = ck->pos.y + j;

			glUniform4f(ctx->gl->uni.clr,
				colors.tile[ck->tiles[i][j]].x,
				colors.tile[ck->tiles[i][j]].y,
				colors.tile[ck->tiles[i][j]].z,
				colors.tile[ck->tiles[i][j]].w
				);

			gen_trans_mat4(&ctx->pos, &ctx->trans);
			glUniformMatrix4fv(ctx->gl->uni.mod, 1, GL_TRUE,
				(float *)ctx->trans.v);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	return ir_cont;
}

static enum iteration_result
render_ent(void *_ctx, void *_e)
{
	struct render_ctx *ctx = _ctx;
	struct ent *e = _e;

	ctx->pos.x = e->pos.x;
	ctx->pos.z = e->pos.y;

	glUniform4f(ctx->gl->uni.clr,
		colors.ent[e->type].x,
		colors.ent[e->type].y,
		colors.ent[e->type].z,
		colors.ent[e->type].w
		);

	gen_trans_mat4(&ctx->pos, &ctx->trans);
	glUniformMatrix4fv(ctx->gl->uni.mod, 1, GL_TRUE, (float *)ctx->trans.v);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	return ir_cont;
}

void
opengl_ui_render(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	double time;
	struct mat4 mview;
	if (cam.changed) {
		gen_look_at(&cam, &mview);
		glUniformMatrix4fv(ctx->uni.view, 1, GL_TRUE, (float *)mview.v);
		cam.changed = false;
	}

	struct render_ctx rctx = { .gl = ctx };

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	L("rendering ents");
	time = glfwGetTime();
	hdarr_for_each(hf->sim->w->ents, &rctx, render_ent);
	L("%f s", glfwGetTime() - time);

	L("rendering chunks");
	time = glfwGetTime();
	hdarr_for_each(hf->sim->w->chunks->hd, &rctx, render_chunk);
	L("%f s", glfwGetTime() - time);

	glfwSwapBuffers(ctx->window);
}

void
opengl_ui_handle_input(struct keymap **km, struct hiface *hf)
{
	glfwPollEvents();

	L("handling input");
	handle_held_keys();
}

struct rectangle
opengl_ui_viewport(struct opengl_ui_ctx *nc)
{
	struct rectangle r = { 0 };

	return r;
}

void
opengl_ui_deinit(void)
{
	glfwTerminate();
}
