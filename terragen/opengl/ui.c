#include "posix.h"

#include <string.h>

#include "shared/opengl/menu.h"
#include "shared/opengl/window.h"
#include "shared/serialize/to_disk.h"
#include "shared/util/log.h"
#include "terragen/gen/write_tiles.h"
#include "terragen/opengl/render/menu.h"
#include "terragen/opengl/render/mesh.h"
#include "terragen/opengl/render/pixels.h"
#include "terragen/opengl/ui.h"
#include "terragen/opengl/worker.h"

#define TICK NS_IN_S / 30

static void
mouse_callback(struct GLFWwindow *win, double x, double y)
{
	struct ui_ctx *ctx = glfwGetWindowUserPointer(win);

	ctx->mousex = x;
	ctx->mousey = y;
}

static void
mouse_button_callback(GLFWwindow* win, int button, int action, int _mods)
{
	struct ui_ctx *ctx = glfwGetWindowUserPointer(win);

	++button;

	if (action == GLFW_PRESS) {
		ctx->mb_pressed |= button;
	} else {
		ctx->mb_pressed &= ~button;
		ctx->mb_released |= button;
	}
}

static bool
genworld_interactive_setup(struct ui_ctx *ctx)
{
	ctx->heightmap_opacity = 0.95f;

	if (!init_window(&ctx->win)) {
		return false;
	} else if (!menu_setup(&ctx->menu_ctx)) {
		return false;
	} else if (!render_mesh_setup(ctx)) {
		return false;
	} else if (!render_pixels_setup(ctx)) {
		return false;
	}

	render_terragen_menu_init(ctx);

	glfwSetWindowUserPointer(ctx->win.win, ctx);
	/* glfwSetKeyCallback(ctx->win.win, key_callback); */
	glfwSetCursorPosCallback(ctx->win.win, mouse_callback);
	/* glfwSetScrollCallback(ctx->win.win, scroll_callback); */
	glfwSetMouseButtonCallback(ctx->win.win, mouse_button_callback);

	glClearColor(1, 1, 1, 1.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	return true;
}

static void
write_file(struct ui_ctx *ctx, const char *outfile)
{
	FILE *f;

	if (strcmp(outfile, "-") == 0) {
		f = stdout;
	} else if (!(f = fopen(outfile, "w"))) {
		LOG_W("unable write to file '%s'", outfile);
	}

	struct chunks chunks;
	chunks_init(&chunks);
	tg_write_tiles(&ctx->ctx, &chunks);

	write_chunks(f, &chunks);
	fclose(f);
	chunks_destroy(&chunks);
}

void
genworld_interactive(terragen_opts opts, const char *outfile)
{
	struct ui_ctx ctx = { 0 };

	if (!genworld_interactive_setup(&ctx)) {
		return;
	}

	memcpy(ctx.opts, opts, sizeof(terragen_opts));
	init_genworld_worker();
	start_genworld_worker(&ctx);

	while (!glfwWindowShouldClose(ctx.win.win)) {
		glViewport(0, 0, ctx.win.px_width, ctx.win.px_height);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		glfwPollEvents();

		render_mesh_setup_frame(&ctx);
		render_mesh(&ctx);

		ctx.dim_changed = false;

		render_pixels_setup_frame(&ctx);
		render_pixels(&ctx);

		render_terragen_menu(&ctx);

		glfwSwapBuffers(ctx.win.win);

		ctx.win.resized = false;
		ctx.mb_released = 0;

		if (ctx.write_file) {
			cancel_genworld_worker();
			write_file(&ctx, outfile);
			ctx.write_file = false;
		}
	}

	glfwTerminate();
}
