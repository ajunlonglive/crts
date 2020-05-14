#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "client/ui/opengl/text.h"
#include "client/ui/opengl/color_cfg.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/input.h"
#include "client/ui/opengl/solids.h"
#include "client/ui/opengl/ui.h"
#include "client/ui/opengl/winutil.h"
#include "shared/math/linalg.h"
#include "shared/sim/ent.h"
#include "shared/types/darr.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

#define FOV 0.47

enum render_categories {
	rcat_chunk = 0,
	rcat_ents = 1,
};

/* Needed for resize_callback */
static struct opengl_ui_ctx *global_ctx;

static void
resize_callback(struct GLFWwindow *win, int width, int height)
{
	mat4 mproj;
	glViewport(0, 0, width, height);

	gen_perspective_mat4(FOV, (float)width / (float)height, 0.1, 1000.0, mproj);

	glUseProgram(global_ctx->chunks.id);
	glUniformMatrix4fv(global_ctx->chunks.uni.proj, 1, GL_TRUE, (float *)mproj);

	update_text_viewport(width, height);

	global_ctx->width = width;
	global_ctx->height = height;
}

static bool
setup_program_chunks(struct opengl_ui_ctx *ctx)
{
	struct shader_src src[] = {
		{ "client/ui/opengl/shaders/shader.vert", GL_VERTEX_SHADER   },
		{ "client/ui/opengl/shaders/shader.frag", GL_FRAGMENT_SHADER },
		{ "\0" }
	};

	ctx->echash = hash_init(2048, 1, sizeof(struct point));

	if (!link_shaders(src, &ctx->chunks.id)) {
		return false;
	}

	ctx->chunks.uni.view      = glGetUniformLocation(ctx->chunks.id, "view");
	ctx->chunks.uni.proj      = glGetUniformLocation(ctx->chunks.id, "proj");
	ctx->chunks.uni.clr       = glGetUniformLocation(ctx->chunks.id, "tile_color");
	ctx->chunks.uni.positions = glGetUniformLocation(ctx->chunks.id, "positions");
	ctx->chunks.uni.types     = glGetUniformLocation(ctx->chunks.id, "types");
	ctx->chunks.uni.cat       = glGetUniformLocation(ctx->chunks.id, "cat");
	ctx->chunks.uni.bases     = glGetUniformLocation(ctx->chunks.id, "bases");
	ctx->chunks.uni.view_pos  = glGetUniformLocation(ctx->chunks.id, "view_pos");

	glGenVertexArrays(1, &ctx->chunks.vao);
	glGenBuffers(1, &ctx->chunks.vbo);

	glBindVertexArray(ctx->chunks.vao);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->chunks.vbo);

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

	return true;
}

struct opengl_ui_ctx *
opengl_ui_init(char *graphics_path)
{
	int x, y;
	struct opengl_ui_ctx *ctx = calloc(1, sizeof(struct opengl_ui_ctx));
	global_ctx = ctx;

	if (!(ctx->window = init_window())) {
		goto free_exit;
	} else if (!setup_program_chunks(ctx)) {
		goto free_exit;
	} else if (!color_cfg(graphics_path, ctx)) {
		goto free_exit;
	}

	/* Set callbacks */
	set_input_callbacks(ctx->window);
	glfwSetFramebufferSizeCallback(ctx->window, resize_callback);
	glfwSetInputMode(ctx->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glUseProgram(ctx->chunks.id);

	text_init();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glfwGetWindowSize(ctx->window, &x, &y);
#ifdef __APPLE__
	/* HACK macOS has a black screen before being resized */

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glfwSwapBuffers(ctx->window);

	x += 1; y += 1;
	glfwSetWindowSize(ctx->window, x, y);
#endif
	global_ctx->width = x;
	global_ctx->height = y;

	return ctx;
free_exit:
	opengl_ui_deinit(ctx);
	return NULL;
}

static void
render_chunks(struct chunks *cnks, struct opengl_ui_ctx *ctx)
{
	int ipos[3] = { 0 };

	uint32_t cat = rcat_chunk;
	glUniform1uiv(ctx->chunks.uni.cat, 1, &cat);

	struct chunk *ck;
	struct point sp = nearest_chunk(&ctx->ref.pos);
	int spy = sp.y,
	    endx = ctx->ref.pos.x + ctx->ref.width,
	    endy = ctx->ref.pos.y + ctx->ref.height;

	for (; sp.x < endx; sp.x += CHUNK_SIZE) {
		for (sp.y = spy; sp.y < endy; sp.y += CHUNK_SIZE) {
			if ((ck = hdarr_get(cnks->hd, &sp))) {
				ipos[0] = ck->pos.x;
				ipos[1] = ck->pos.y;

				glUniform3iv(ctx->chunks.uni.positions, 1, ipos);
				glUniform1uiv(ctx->chunks.uni.types, 256,
					(uint32_t *)&ck->tiles);

				/* draw on extra for the chunk's base */
				glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 256 + 1);
			}
		}
	}
}

static void
render_ents(struct hdarr *ents, struct hdarr *cnks, struct opengl_ui_ctx *ctx)
{
	struct ent *emem = darr_raw_memory(hdarr_darr(ents));
	size_t i, j, len = hdarr_len(ents);

	hash_clear(ctx->echash);

	uint32_t cat = rcat_ents;
	glUniform1uiv(ctx->chunks.uni.cat, 1, &cat);

	/*
	   struct chunk *ck;
	   struct point p;
	 */

	int32_t positions[256 * 3] = { 0 };
	uint32_t types[256] = { 0 };
	//uint32_t bases[256];
	const size_t *st;

	for (i = 0, j = 0; i < len; ++i, ++j) {
		if (i >= 256) {
			glUniform1uiv(ctx->chunks.uni.types, 256, types);
			glUniform3iv(ctx->chunks.uni.positions, 256, positions);
			glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 256);
			j = 0;
		}

		positions[(j * 3) + 0] = emem[i].pos.x;
		positions[(j * 3) + 1] = emem[i].pos.y;

		if ((st = hash_get(ctx->echash, &emem[i].pos))) {
			positions[(j * 3) + 2] = *st + 1;
			hash_set(ctx->echash, &emem[i].pos, *st + 1);
		} else {
			positions[(j * 3) + 2] = 0;
			hash_set(ctx->echash, &emem[i].pos, 0);
		}

		types[j] = emem[i].type;

		/*
		   p = nearest_chunk(&emem[i].pos);
		   if ((ck = hdarr_get(cnks, &p))) {
		        p = point_sub(&emem[i].pos, &p);
		        bases[i] = ck->tiles[p.x][p.y];
		   } else {
		        bases[i] = tile_deep_water;
		   }
		 */
	}
	glUniform1uiv(ctx->chunks.uni.types, 256, types);
	glUniform3iv(ctx->chunks.uni.positions, 256, positions);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, len % 256);
}

void
render_world(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	glUseProgram(ctx->chunks.id);
	glBindVertexArray(ctx->chunks.vao);

	if (cam.changed) {
		mat4 mview;

		cam.tgt[0] = cos(cam.yaw) * cos(cam.pitch);
		cam.tgt[1] = sin(cam.pitch);
		cam.tgt[2] = sin(cam.yaw) * cos(cam.pitch);

		gen_look_at(&cam, mview);
		glUniformMatrix4fv(ctx->chunks.uni.view, 1, GL_TRUE, (float *)mview);
		glUniform3fv(ctx->chunks.uni.view_pos, 1, cam.pos);
		cam.changed = false;
	}

	render_chunks(hf->sim->w->chunks, ctx);

	render_ents(hf->sim->w->ents, hf->sim->w->chunks->hd, ctx);
}

void
opengl_ui_render(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	static double ftime = 0.0, setup = 0.0, render = 0.0;
	double start = glfwGetTime(), stop;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render_world(ctx, hf);

	text_setup_render();

	gl_printf(0, -1, "t: %.2fms (%.1f fps) | s: %.1f%%, r: %.1f%%",
		ftime * 1000,
		1 / ftime,
		100 * setup / ftime,
		100 * render / ftime);
	gl_printf(0, -2, "cam: %.2f,%.2f,%.2f p: %.1f y: %.1f",
		cam.pos[0],
		cam.pos[1],
		cam.pos[2],
		cam.pitch  * (180.0f / PI),
		cam.yaw * (180.0f / PI));

	setup = glfwGetTime() - start;

	glfwSwapBuffers(ctx->window);

	stop = glfwGetTime();
	render = stop - setup - start;
	ftime = stop - start;
}

void
opengl_ui_handle_input(struct opengl_ui_ctx *ctx, struct keymap **km,
	struct hiface *hf)
{
	glfwPollEvents();

	handle_held_keys(hf, km);
	handle_gl_mouse(hf);

	float off = cam.pos[1] * tan(FOV);

	ctx->ref.pos.x = hf->view.x;
	ctx->ref.pos.y = hf->view.y;
	ctx->ref.width = (ctx->width + off * 2) / 10;
	ctx->ref.height = (ctx->height + off * 2) / 10;

	cam.pos[0] = (float)hf->view.x;
	cam.pos[2] = (float)hf->view.y;

	cam.changed = true;

	if (glfwWindowShouldClose(ctx->window)) {
		hf->sim->run = false;
	} else if (!hf->sim->run) {
		glfwSetWindowShouldClose(ctx->window, 1);
	}
}

struct rectangle
opengl_ui_viewport(struct opengl_ui_ctx *nc)
{
	return nc->ref;
}

void
opengl_ui_deinit(struct opengl_ui_ctx *ctx)
{
	hash_destroy(ctx->echash);
	free(ctx);
	glfwTerminate();
}
