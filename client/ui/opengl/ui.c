#include <math.h>
#include <stdlib.h>

#include "client/ui/opengl/ui.h"
#include "client/ui/opengl/winutil.h"
#include "shared/math/linalg.h"
#include "shared/util/log.h"

struct opengl_ui_ctx {
	GLFWwindow* window;
	uint32_t prog_id, vao, vbo;
	struct {
		uint32_t mod, view, proj;
	} uni;
};

static struct camera cam = {
	{ 0, 0, -3, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	0, 0
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

struct vec4 cubePositions[] = {
	{  0.0f,  0.0f,  0.0f },
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
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_S:
			cam.pos = vec4_add(cam.pos, vec4_scale(cam.tgt, 0.5));
			break;
		case GLFW_KEY_W:
			cam.pos = vec4_sub(cam.pos, vec4_scale(cam.tgt, 0.5));
			break;
		case GLFW_KEY_A:
			cam.pos = vec4_add(cam.pos, vec4_scale(vec4_normalize(
				vec4_cross(cam.tgt, cam.up)), 0.5));
			break;
		case GLFW_KEY_D:
			cam.pos = vec4_sub(cam.pos, vec4_scale(vec4_normalize(
				vec4_cross(cam.tgt, cam.up)), 0.5));
			break;
		}
	}
}

void
mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	static double lastx = 0, lasty = 0;

	cam.yaw   += (xpos - lastx) * 0.0026646259971648;
	cam.pitch += (ypos - lasty) * 0.0026646259971648;

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

	glGenVertexArrays(1, &ctx->vao);
	glGenBuffers(1, &ctx->vbo);

	glBindVertexArray(ctx->vao);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	return ctx;

free_exit:
	free(ctx);
	return NULL;
}

void
opengl_ui_render(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(ctx->prog_id);

	struct mat4 mview = gen_look_at(cam);
	glUniformMatrix4fv(ctx->uni.view, 1, GL_TRUE, (float *)mview.v);

	struct mat4 mproj = gen_perspective_mat4(fov, 800.0f / 600.0f, 0.1, 1000.0);
	glUniformMatrix4fv(ctx->uni.proj, 1, GL_TRUE, (float *)mproj.v);

	glBindVertexArray(ctx->vao);
	for (int i = 0; i < 10; ++i) {
		struct mat4 mmodel = gen_trans_mat4(cubePositions[i]);
		glUniformMatrix4fv(ctx->uni.mod, 1, GL_TRUE, (float *)mmodel.v);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	glfwSwapBuffers(ctx->window);
}

void
opengl_ui_handle_input(struct keymap **km, struct hiface *hf)
{
	glfwPollEvents();
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
