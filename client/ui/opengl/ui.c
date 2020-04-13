#include <assert.h>
#include <math.h>
#include <stdlib.h>

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

enum modifier_types {
	mod_shift = 1 << 0,
};

struct {
	uint8_t held[0xff];
	uint8_t mod;
} keyboard = { 0 };

struct vec4 cubePositions[] = {
	{   0.0f,  0.0f,  0.0f },
	{   2.0f,  5.0f, -15.0f },
	{  -1.5f, -2.2f, -2.5f },
	{  -3.8f, -2.0f, -12.3f },
	{   2.4f, -0.4f, -3.5f },
	{  -1.7f,  3.0f, -7.5f },
	{   1.3f, -2.0f, -2.5f },
	{   1.5f,  2.0f, -2.5f },
	{   1.5f,  0.2f, -1.5f },
	{  -1.3f,  1.0f, -1.5f }
};

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

struct opengl_ui_ctx *
opengl_ui_init(void)
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

struct vec4 ent_colors[ent_type_count] = {
	[et_worker] = { 0.9, 0.4, 0.1, 1.0 },
	[et_elf_corpse] = { 0.1, 0.2, 0.1, 1.0 },
	[et_deer] = { 0.3, 0.9, 0.9, 1.0 },
	[et_fish] = { 0.3, 0.1, 0.1, 1.0 },
};

static enum iteration_result
render_ent(void *_ctx, void *_e)
{
	struct opengl_ui_ctx *ctx = _ctx;
	struct ent *e = _e;

	struct vec4 epos = { e->pos.x, 0, e->pos.y };

	glUniform4f(ctx->uni.clr,
		ent_colors[e->type].x,
		ent_colors[e->type].y,
		ent_colors[e->type].z,
		ent_colors[e->type].w
		);

	struct mat4 mmodel = gen_trans_mat4(epos);
	glUniformMatrix4fv(ctx->uni.mod, 1, GL_TRUE, (float *)mmodel.v);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	return ir_cont;
}

void
opengl_ui_render(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	int width, height;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(ctx->prog_id);

	glfwGetFramebufferSize(ctx->window, &width, &height);
	glViewport(0, 0, width, height);

	struct mat4 mproj = gen_perspective_mat4(fov,
		(float)width / (float)height, 0.1, 1000.0);

	glUniformMatrix4fv(ctx->uni.proj, 1, GL_TRUE, (float *)mproj.v);

	struct mat4 mview = gen_look_at(cam);
	glUniformMatrix4fv(ctx->uni.view, 1, GL_TRUE, (float *)mview.v);

	glBindVertexArray(ctx->vao);
	hdarr_for_each(hf->sim->w->ents, ctx, render_ent);

	glfwSwapBuffers(ctx->window);
}

void
opengl_ui_handle_input(struct keymap **km, struct hiface *hf)
{
	size_t i;
	glfwPollEvents();

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
			cam.pos = vec4_add(cam.pos, vec4_scale(cam.tgt, speed));
			break;
		case GLFW_KEY_W:
			cam.pos = vec4_sub(cam.pos, vec4_scale(cam.tgt, speed));
			break;
		case GLFW_KEY_A:
			cam.pos = vec4_add(cam.pos, vec4_scale(vec4_normalize(
				vec4_cross(cam.tgt, cam.up)), speed));
			break;
		case GLFW_KEY_D:
			cam.pos = vec4_sub(cam.pos, vec4_scale(vec4_normalize(
				vec4_cross(cam.tgt, cam.up)), speed));
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
