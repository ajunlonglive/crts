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

static char *err_vals[] = {
	[GL_INVALID_ENUM]                  = "INVALID_ENUM",
	[GL_INVALID_VALUE]                 = "INVALID_VALUE",
	[GL_INVALID_OPERATION]             = "INVALID_OPERATION",
	[GL_STACK_OVERFLOW]                = "STACK_OVERFLOW",
	[GL_STACK_UNDERFLOW]               = "STACK_UNDERFLOW",
	[GL_OUT_OF_MEMORY]                 = "OUT_OF_MEMORY",
	[GL_INVALID_FRAMEBUFFER_OPERATION] = "INVALID_FRAMEBUFFER_OPERATION",
};

#define GLCHECK(code) while ((code = glGetError()) != GL_NO_ERROR) L("GL error: %s", err_vals[code])

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
	mat4 mproj;
	int32_t code;
	glViewport(0, 0, width, height);
	GLCHECK(code);

	gen_perspective_mat4(0.47, (float)width / (float)height, 0.1, 1000.0, mproj);
	glUniformMatrix4fv(global_ctx->uni.proj, 1, GL_TRUE, (float *)mproj);
	GLCHECK(code);
}

void GLAPIENTRY
gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);

}

struct opengl_ui_ctx *
opengl_ui_init(char *graphics_path)
{
	struct opengl_ui_ctx *ctx = calloc(1, sizeof(struct opengl_ui_ctx));
	global_ctx = ctx;
	int32_t code;

	struct shader_src prog1[] = {
		{ "client/ui/opengl/shaders/shader.vert",   GL_VERTEX_SHADER   },
		{ "client/ui/opengl/shaders/shader.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	if (!(ctx->window = init_window())) {
		goto free_exit;
	} else if (!link_shaders(prog1, &ctx->prog_id)) {
		goto free_exit;
	}

	GLCHECK(code);

	if (!color_cfg(graphics_path, ctx)) {
		goto free_exit;
	}
	GLCHECK(code);

	/* Set callbacks */
	set_input_callbacks(ctx->window);
	glfwSetFramebufferSizeCallback(ctx->window, resize_callback);

	glfwSetInputMode(ctx->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	GLCHECK(code);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		GLCHECK(code);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		GLCHECK(code);
		glDebugMessageCallback(gl_debug_callback, 0);
		GLCHECK(code);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
		GLCHECK(code);
	}

	/* wireframe mode */
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	GLCHECK(code);
	/*glEnable(GL_CULL_FACE);
	   glCullFace(GL_FRONT);*/

	ctx->uni.mod  = glGetUniformLocation(ctx->prog_id, "model");
	GLCHECK(code);
	ctx->uni.view = glGetUniformLocation(ctx->prog_id, "view");
	GLCHECK(code);
	ctx->uni.proj = glGetUniformLocation(ctx->prog_id, "proj");
	GLCHECK(code);
	ctx->uni.clr  = glGetUniformLocation(ctx->prog_id, "tile_colors");
	GLCHECK(code);

	glGenVertexArrays(1, &ctx->vao);
	glGenBuffers(1, &ctx->vbo);

	glBindVertexArray(ctx->vao);
	GLCHECK(code);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
	GLCHECK(code);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * solid_cube.len, solid_cube.verts, GL_STATIC_DRAW);
	GLCHECK(code);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	GLCHECK(code);
	glEnableVertexAttribArray(0);
	GLCHECK(code);

	/* TODO: right now we only have one of each of these */
	glUseProgram(ctx->prog_id);
	GLCHECK(code);
	glBindVertexArray(ctx->vao);
	GLCHECK(code);

	return ctx;

free_exit:
	free(ctx);
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

enum iteration_result
render_ent(void *_ctx, void *_e)
{
	struct render_ctx *ctx = _ctx;
	struct ent *e = _e;
	int32_t code;

	ctx->pos[0] = e->pos.x;
	ctx->pos[2] = e->pos.y;

	glUniform4f(ctx->gl->uni.clr,
		colors.ent[e->type][0],
		colors.ent[e->type][1],
		colors.ent[e->type][2],
		colors.ent[e->type][3]
		);
	GLCHECK(code);

	gen_trans_mat4(ctx->pos, ctx->trans);
	glUniformMatrix4fv(ctx->gl->uni.mod, 1, GL_TRUE, (float *)ctx->trans);
	GLCHECK(code);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	GLCHECK(code);

	return ir_cont;
}

static void
render_chunks(struct chunks *cnks, struct opengl_ui_ctx *ctx)
{
	struct chunk *cmem = darr_raw_memory(hdarr_darr(cnks->hd));
	size_t ci, len = hdarr_len(cnks->hd);
	uint16_t i, j, code;
	vec4 pos = { 0 };
	mat4 trans = { 0 };

	//glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 7, GL_DEBUG_SEVERITY_NOTIFICATION, 4, "test");

	for (ci = 0; ci < len; ++ci) {
		for (i = 0; i < CHUNK_SIZE; ++i) {
			for (j = 0; j < CHUNK_SIZE; ++j) {
				pos[0] = cmem[ci].pos.x + i;
				pos[1] = tile_heights[cmem[ci].tiles[i][j]] - 1;
				pos[2] = cmem[ci].pos.y + j;

				/*
				   glUniform4f(ctx->uni.clr,
				        colors.tile[cmem[ci].tiles[i][j]][0],
				        colors.tile[cmem[ci].tiles[i][j]][1],
				        colors.tile[cmem[ci].tiles[i][j]][2],
				        colors.tile[cmem[ci].tiles[i][j]][3]
				        );
				 */

				gen_trans_mat4(pos, trans);
				glUniformMatrix4fv(ctx->uni.mod, 1, GL_TRUE, (float *)trans);
				GLCHECK(code);
				glDrawArrays(GL_TRIANGLES, 0, 36);
				GLCHECK(code);
			}
		}
	}
}

void
opengl_ui_render(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	mat4 mview;
	int32_t code;
	if (cam.changed) {
		gen_look_at(&cam, mview);
		glUniformMatrix4fv(ctx->uni.view, 1, GL_TRUE, (float *)mview);
		GLCHECK(code);
		cam.changed = false;
	}

	//struct render_ctx rctx = { .gl = ctx };

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	GLCHECK(code);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLCHECK(code);

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
opengl_ui_deinit(void)
{
	glfwTerminate();
}
