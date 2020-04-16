#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "client/cfg/common.h"
#include "client/cfg/graphics.h"
#include "client/ui/graphics.h"
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

#define DEG_90 1.57f

static struct camera cam = {
	{ 0, 255, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	.yaw = 0.07, .pitch = DEG_90
};

float fov = 0.47;

float cube[] = {
	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,

	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,

	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,

	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f,  0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
};

float square[] = {
	0.5f,  0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	-0.5f,  0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	-0.5f, -0.5f, 0.0f,
	-0.5f,  0.5f, 0.0f
};

enum modifier_types {
	mod_shift = 1 << 0,
};

struct {
	uint8_t held[0xff];
	uint8_t mod;
} keyboard = { 0 };

static void
key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key < 0xff && key > 0) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			keyboard.held[key] = 1;
		} else {
			keyboard.held[key] = 0;
		}
	} else if (key == GLFW_KEY_LEFT_SHIFT) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			keyboard.mod |= mod_shift;
		} else {
			keyboard.mod &= ~mod_shift;
		}
	}
}

void
mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	static double lastx = 0, lasty = 0;

	cam.yaw   += (xpos - lastx) * 0.0026646259971648;
	cam.pitch += (ypos - lasty) * 0.0026646259971648;

	/*
	   if (cam.yaw > DEG_90) {
	        cam.yaw = DEG_90;
	   } else if (cam.yaw < -DEG_90) {
	        cam.yaw = -DEG_90;
	   }
	 */

	if (cam.pitch > DEG_90) {
		cam.pitch = DEG_90;
	} else if (cam.pitch < -DEG_90) {
		cam.pitch = -DEG_90;
	}

	L("yaw: %f, pitch: %f", cam.yaw, cam.pitch);

	lastx = xpos;
	lasty = ypos;

	cam.tgt.x = cos(cam.yaw) * cos(cam.pitch);
	cam.tgt.y = sin(cam.pitch);
	cam.tgt.z = sin(cam.yaw) * cos(cam.pitch);
}

struct vec4 ent_colors[extended_ent_type_count] = { 0 };
struct vec4 tile_colors[tile_count] = { 0 };
struct vec4 grayscale_colors[] = {
	[0]  = { 0.0703125, 0.0703125, 0.0703125, 1.0 },
	[1]  = { 0.109375, 0.109375, 0.109375, 1.0 },
	[2]  = { 0.1484375, 0.1484375, 0.1484375, 1.0 },
	[3]  = { 0.1875, 0.1875, 0.1875, 1.0 },
	[4]  = { 0.2265625, 0.2265625, 0.2265625, 1.0 },
	[5]  = { 0.265625, 0.265625, 0.265625, 1.0 },
	[6]  = { 0.3046875, 0.3046875, 0.3046875, 1.0 },
	[7]  = { 0.34375, 0.34375, 0.34375, 1.0 },
	[8]  = { 0.3828125, 0.3828125, 0.3828125, 1.0 },
	[9]  = { 0.421875, 0.421875, 0.421875, 1.0 },
	[10] = { 0.4609375, 0.4609375, 0.4609375, 1.0 },
	[11] = { 0.5, 0.5, 0.5, 1.0 },
	[12] = { 0.5390625, 0.5390625, 0.5390625, 1.0 },
	[13] = { 0.578125, 0.578125, 0.578125, 1.0 },
	[14] = { 0.6171875, 0.6171875, 0.6171875, 1.0 },
	[15] = { 0.65625, 0.65625, 0.65625, 1.0 },
	[16] = { 0.6953125, 0.6953125, 0.6953125, 1.0 },
	[17] = { 0.734375, 0.734375, 0.734375, 1.0 },
	[18] = { 0.7734375, 0.7734375, 0.7734375, 1.0 },
	[19] = { 0.8125, 0.8125, 0.8125, 1.0 },
	[20] = { 0.8515625, 0.8515625, 0.8515625, 1.0 },
	[21] = { 0.890625, 0.890625, 0.890625, 1.0 },
	[22] = { 0.9296875, 0.9296875, 0.9296875, 1.0 },
};

float tile_heights[tile_count] = {
	[tile_mountain] = 1,
	[tile_peak] = 2,
	[tile_water] = -1,
	[tile_deep_water] = -2,
};

static bool
setup_color(void *_, int32_t sect, int32_t type,
	char c, short fg, short bg, short attr, short zi)
{
	int r, g, b;
	float rf, gf, bf;

	if (fg >= 233) {
		fg -= 233;
		rf = grayscale_colors[fg].x;
		gf = grayscale_colors[fg].y;
		bf = grayscale_colors[fg].z;
	} else {
		fg -= 16;

		r = fg / 36;
		g = (fg % 36) / 6;
		b = (fg % 36) % 6;

		rf = r * 0.16666666666666666;
		gf = g * 0.16666666666666666;
		bf = b * 0.16666666666666666;
	}

	switch (sect) {
	case gfx_cfg_section_entities:
		ent_colors[type].x = rf;
		ent_colors[type].y = gf;
		ent_colors[type].z = bf;
		break;
	case gfx_cfg_section_tiles:
		tile_colors[type].x = rf;
		tile_colors[type].y = gf;
		tile_colors[type].z = bf;
		break;
	}

	return true;
}

struct opengl_ui_ctx *
opengl_ui_init(char *graphics_path)
{
	struct opengl_ui_ctx *ctx = calloc(1, sizeof(struct opengl_ui_ctx));

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

	struct parse_graphics_ctx cfg_ctx = { NULL, setup_color };
	if (!parse_cfg_file(graphics_path, &cfg_ctx, parse_graphics_handler)) {
		goto free_exit;
	}

	glfwSetKeyCallback(ctx->window, key_callback);
	glfwSetCursorPosCallback(ctx->window, mouse_callback);
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	/* Set mouse target */
	mouse_callback(ctx->window, 0, 0);

	return ctx;

free_exit:
	free(ctx);
	return NULL;
}

static enum iteration_result
render_chunk(void *_ctx, void *_ck)
{
	struct opengl_ui_ctx *ctx = _ctx;
	struct chunk *ck = _ck;
	struct mat4 mmodel;

	uint16_t i, j;

	struct vec4 tpos = { 0, 0, 0, 0 };

	for (i = 0; i < CHUNK_SIZE; ++i) {
		for (j = 0; j < CHUNK_SIZE; ++j) {
			tpos.x = ck->pos.x + i;
			tpos.y = tile_heights[ck->tiles[i][j]] - 1;
			tpos.z = ck->pos.y + j;

			glUniform4f(ctx->uni.clr,
				tile_colors[ck->tiles[i][j]].x,
				tile_colors[ck->tiles[i][j]].y,
				tile_colors[ck->tiles[i][j]].z,
				tile_colors[ck->tiles[i][j]].w
				);

			gen_trans_mat4(&tpos, &mmodel);
			glUniformMatrix4fv(ctx->uni.mod, 1, GL_TRUE, (float *)mmodel.v);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	return ir_cont;
}

static enum iteration_result
render_ent(void *_ctx, void *_e)
{
	struct opengl_ui_ctx *ctx = _ctx;
	struct ent *e = _e;
	struct mat4 mmodel;

	struct vec4 epos = { e->pos.x, 0, e->pos.y };

	glUniform4f(ctx->uni.clr,
		ent_colors[e->type].x,
		ent_colors[e->type].y,
		ent_colors[e->type].z,
		ent_colors[e->type].w
		);

	gen_trans_mat4(&epos, &mmodel);
	glUniformMatrix4fv(ctx->uni.mod, 1, GL_TRUE, (float *)mmodel.v);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	return ir_cont;
}

void
opengl_ui_render(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	int width, height;
	struct mat4 mproj, mview;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(ctx->prog_id);

	glfwGetFramebufferSize(ctx->window, &width, &height);
	glViewport(0, 0, width, height);

	gen_perspective_mat4(fov, (float)width / (float)height, 0.1, 1000.0, &mproj);
	glUniformMatrix4fv(ctx->uni.proj, 1, GL_TRUE, (float *)mproj.v);

	gen_look_at(&cam, &mview);
	glUniformMatrix4fv(ctx->uni.view, 1, GL_TRUE, (float *)mview.v);

	glBindVertexArray(ctx->vao);

	hdarr_for_each(hf->sim->w->ents, ctx, render_ent);

	hdarr_for_each(hf->sim->w->chunks->hd, ctx, render_chunk);

	glfwSwapBuffers(ctx->window);
}

void
opengl_ui_handle_input(struct keymap **km, struct hiface *hf)
{
	size_t i;
	glfwPollEvents();
	struct vec4 v1;

	float speed = 0.5;

	for (i = 0; i < 0xff; ++i) {
		if (!keyboard.held[i]) {
			continue;
		}

		if (keyboard.mod & mod_shift) {
			speed = 2.0;
		}

		switch (i) {
		case GLFW_KEY_S:
			v1 = cam.tgt;
			vec4_scale(&v1, speed);
			vec4_add(&cam.pos, &v1);
			break;
		case GLFW_KEY_W:
			v1 = cam.tgt;
			vec4_scale(&v1, speed);
			vec4_sub(&cam.pos, &v1);
			break;
		case GLFW_KEY_A:
			v1 = cam.tgt;
			vec4_cross(&v1, &cam.up);
			vec4_normalize(&v1);
			vec4_scale(&v1, speed);
			vec4_add(&cam.pos, &v1);
			break;
		case GLFW_KEY_D:
			v1 = cam.tgt;
			vec4_cross(&v1, &cam.up);
			vec4_normalize(&v1);
			vec4_scale(&v1, speed);
			vec4_sub(&cam.pos, &v1);
			break;
		default:
			break;
		}
	}
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
