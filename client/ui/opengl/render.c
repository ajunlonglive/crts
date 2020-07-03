#include "posix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef OPENGL_UI
#define OPENGL_UI
#endif

#include "client/opts.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/loaders/obj.h"
#include "client/ui/opengl/render.h"
#include "client/ui/opengl/render/chunks.h"
#include "client/ui/opengl/render/ents.h"
#include "client/ui/opengl/render/hud.h"
#include "client/ui/opengl/render/selection.h"
#include "client/ui/opengl/render/shadows.h"
#include "client/ui/opengl/render/text.h"
#include "shared/util/log.h"

static struct hdarr *chunk_meshes;
static struct shadow_map shadow_map = { .dim = 2048 };

bool
opengl_ui_render_setup(struct c_opts *opts)
{
	obj_loader_setup();

	return render_world_setup_shadows(&shadow_map)
	       && render_world_setup_ents()
	       && render_world_setup_chunks(&chunk_meshes)
	       && render_world_setup_selection()
	       && render_text_setup(opts->opengl_ui_scale);
}

void
opengl_ui_render_teardown(void)
{
	hdarr_destroy(chunk_meshes);
}

static void
render_everything(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	/* ents */
	render_ents(hf, ctx);

	/* selection */
	if (ctx->pass == rp_final) {
		render_selection(hf, ctx, chunk_meshes);
	}

	/* chunks */
	render_chunks(hf, ctx, chunk_meshes);
}

static void
render_world(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	float w, h;
	static struct rectangle oref = { 0 };

	if (cam.changed || ctx->resized || !points_equal(&hf->view, &ctx->ref.pos)) {
		ctx->ref.pos = hf->view;

		if (!cam.unlocked) {
			w = cam.pos[1] * (float)ctx->width / (float)ctx->height * 0.48;
			h = cam.pos[1] * tanf(FOV / 2) * 2;
			cam.pos[0] = ctx->ref.pos.x + w;
			cam.pos[2] = ctx->ref.pos.y + h * 2;
			/* TODO: calculate this value more conservatively? */
			ctx->ref.width = w * 2;
			ctx->ref.height = h * 2;
		}

		cam_calc_tgt(&cam);

		gen_look_at(&cam, ctx->mview);

		mat4_mult_mat4(ctx->mproj, ctx->mview,  ctx->mviewproj);

		cam.changed = true;

		if ((ctx->ref_changed = memcmp(&oref, &ctx->ref, sizeof(struct rectangle)))) {
			oref = ctx->ref;
		}
	}

	glViewport(0, 0, ctx->width, ctx->height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	render_everything(ctx, hf);

	/* last usage of cam.changed */
	cam.changed = ctx->ref_changed = false;
}

void
opengl_ui_render(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	static double last_start = 0.0;

	double start = glfwGetTime(), stop;

	ctx->pulse += start - last_start;
	if (ctx->pulse > 2 * PI) {
		ctx->pulse -= 2 * PI;
	}

	render_world(ctx, hf);

	render_hud(ctx, hf);

	if (ctx->debug_hud) {
		render_debug_hud(ctx, hf);
	}

	ctx->prof.setup = glfwGetTime() - start;

	glfwSwapBuffers(ctx->window);

	stop = glfwGetTime();
	ctx->prof.render = stop - ctx->prof.setup - start;
	ctx->prof.ftime = stop - start;
	last_start = start;

	ctx->resized = false;
}
