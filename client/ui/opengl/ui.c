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
#include "shared/types/darr.h"
#include "shared/util/log.h"

/* Needed for resize_callback */
static struct opengl_ui_ctx *global_ctx;

static void
resize_callback(struct GLFWwindow *win, int width, int height)
{
	mat4 mproj;
	glViewport(0, 0, width, height);

	gen_perspective_mat4(0.47, (float)width / (float)height, 0.1, 1000.0, mproj);
	glUniformMatrix4fv(global_ctx->uni.proj, 1, GL_TRUE, (float *)mproj);
}

struct opengl_ui_ctx *
opengl_ui_init(char *graphics_path)
{
	struct opengl_ui_ctx *ctx = calloc(1, sizeof(struct opengl_ui_ctx));
	global_ctx = ctx;

	struct shader_src prog1[] = {
		{ "client/ui/opengl/shaders/shader.vert", GL_VERTEX_SHADER   },
		{ "client/ui/opengl/shaders/shader.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!(ctx->window = init_window())) {
		goto free_exit;
	}

	if (!link_shaders(prog1, &ctx->prog_id)) {
		goto free_exit;
	}

	glUseProgram(ctx->prog_id);

	ctx->uni.view     = glGetUniformLocation(ctx->prog_id, "view");
	ctx->uni.proj     = glGetUniformLocation(ctx->prog_id, "proj");
	ctx->uni.clr      = glGetUniformLocation(ctx->prog_id, "tile_color");
	ctx->uni.corner   = glGetUniformLocation(ctx->prog_id, "corner");
	ctx->uni.tiles    = glGetUniformLocation(ctx->prog_id, "tiles");
	ctx->uni.view_pos = glGetUniformLocation(ctx->prog_id, "view_pos");

	if (!color_cfg(graphics_path, ctx)) {
		goto free_exit;
	}

	/* Set callbacks */
	set_input_callbacks(ctx->window);
	glfwSetFramebufferSizeCallback(ctx->window, resize_callback);
	glfwSetInputMode(ctx->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenVertexArrays(1, &ctx->vao);
	glGenBuffers(1, &ctx->vbo);

	glBindVertexArray(ctx->vao);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);

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

	/* TODO: right now we only have one of each of these */
	glBindVertexArray(ctx->vao);

	return ctx;

free_exit:
	opengl_ui_deinit(ctx);
	return NULL;
}

struct render_ctx {
	vec4 pos;
	mat4 trans;
	struct opengl_ui_ctx *gl;
};

/*
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
 */

/*
   static enum iteration_result
   render_ent(void *_ctx, void *_e)
   {
        struct render_ctx *ctx = _ctx;
        struct ent *e = _e;

        ctx->pos[0] = e->pos.x;
        ctx->pos[2] = e->pos.y;

        glUniform4f(ctx->gl->uni.clr,
                colors.ent[e->type][0],
                colors.ent[e->type][1],
                colors.ent[e->type][2],
                colors.ent[e->type][3]
                );

        gen_trans_mat4(ctx->pos, ctx->trans);
        glUniformMatrix4fv(ctx->gl->uni.mod, 1, GL_TRUE, (float *)ctx->trans);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        return ir_cont;
   }
 */

static void
render_chunks(struct chunks *cnks, struct opengl_ui_ctx *ctx)
{
	struct chunk *cmem = darr_raw_memory(hdarr_darr(cnks->hd));
	size_t ci, len = hdarr_len(cnks->hd);

	int ipos[2] = { 0 };

	for (ci = 0; ci < len; ++ci) {
		ipos[0] = cmem[ci].pos.x;
		ipos[1] = cmem[ci].pos.y;

		glUniform2iv(ctx->uni.corner, 1, ipos);
		glUniform1uiv(ctx->uni.tiles, 256, (uint32_t *)&cmem[ci].tiles);

		/* draw on extra for the chunk's base */
		glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 256 + 1);
	}
}

void
opengl_ui_render(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	mat4 mview;
	if (cam.changed) {
		gen_look_at(&cam, mview);
		glUniformMatrix4fv(ctx->uni.view, 1, GL_TRUE, (float *)mview);
		glUniform3fv(ctx->uni.view_pos, 1, cam.pos);
		cam.changed = false;
	}

	//struct render_ctx rctx = { .gl = ctx };

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//hdarr_for_each(hf->sim->w->ents, &rctx, render_ent);

	//double time_start = glfwGetTime();
	render_chunks(hf->sim->w->chunks, ctx);
	//L("fps: %f", 1.0 / (glfwGetTime() - time_start));

	glfwSwapBuffers(ctx->window);
}

void
opengl_ui_handle_input(struct keymap **km, struct hiface *hf)
{
	glfwPollEvents();
	handle_held_keys(hf);
}

struct rectangle
opengl_ui_viewport(struct opengl_ui_ctx *nc)
{
	struct rectangle r = { { 0, 0 }, 128, 128 };

	return r;
}

void
opengl_ui_deinit(struct opengl_ui_ctx *ctx)
{
	free(ctx);
	glfwTerminate();
}
