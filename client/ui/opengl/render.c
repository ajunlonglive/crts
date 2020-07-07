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
static struct shadow_map shadow_map;

bool
opengl_ui_render_setup(struct opengl_ui_ctx *ctx)
{
	obj_loader_setup();

	cam.pitch = ctx->opts.cam_pitch;
	cam.yaw   = ctx->opts.cam_yaw;

	sun.width = sun.height = ctx->opts.shadow_map_res;
	if (ctx->opts.shadows) {
		shadow_map.dim = ctx->opts.shadow_map_res;
		render_world_setup_shadows(&shadow_map);
	}

	return render_world_setup_ents()
	       && render_world_setup_chunks(&chunk_meshes)
	       && render_world_setup_selection()
	       && render_text_setup(ctx->opts.font_scale);
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
render_setup_frame(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	/* ents */
	render_ents_setup_frame(hf, ctx);

	/* selection */
	render_selection_setup_frame(hf, ctx, chunk_meshes);

	/* chunks */
	render_chunks_setup_frame(hf, ctx, chunk_meshes);
}

static void
render_world(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	float w, h;
	static struct rectangle oref = { 0 };

	if (cam.changed || ctx->resized || !points_equal(&hf->view, &ctx->ref.pos)) {
		ctx->ref.pos = hf->view;

		cam.width = ctx->width;
		cam.height = ctx->height;

		if (!cam.unlocked) {
			w = cam.pos[1] * (float)ctx->width / (float)ctx->height * 0.48;
			h = cam.pos[1] * tanf(FOV / 2) * 2;
			cam.pos[0] = ctx->ref.pos.x + w;
			cam.pos[2] = ctx->ref.pos.y + h * 2;
			/* TODO: calculate this value more conservatively? */
			ctx->ref.width = w * 2;
			ctx->ref.height = h * 2;

			/* update sun position */
			sun.pos[0] = ctx->ref.pos.x + w * 2;
			sun.pos[2] = ctx->ref.pos.y + h;
		}



		cam_calc_tgt(&cam);
		cam_calc_tgt(&sun);

		cam.changed = sun.changed = true;

		if ((ctx->ref_changed = memcmp(&oref, &ctx->ref, sizeof(struct rectangle)))) {
			oref = ctx->ref;
		}
	}

	render_setup_frame(ctx, hf);

	if (ctx->opts.shadows) {
		/* depth pass */
		ctx->pass = rp_depth;

		glViewport(0, 0, shadow_map.dim, shadow_map.dim);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_map.depth_map_fb);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		render_everything(ctx, hf);

		glBindTexture(GL_TEXTURE_2D, shadow_map.depth_map_tex);
		glActiveTexture(GL_TEXTURE0);
	} else {
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
	}

	/* final pass */
	ctx->pass = rp_final;

	glViewport(0, 0, ctx->width, ctx->height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_BACK);

	render_everything(ctx, hf);

	/* last usage of cam.changed */
	sun.changed = cam.changed = ctx->ref_changed = false;
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

	ctx->prof.smo_vert_count  = 0;
	ctx->prof.chunk_count = 0;
}
