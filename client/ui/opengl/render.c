#include "posix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef OPENGL_UI
#define OPENGL_UI
#endif

#include "client/opts.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/input.h"
#include "client/ui/opengl/render.h"
#include "client/ui/opengl/render/chunks.h"
#include "client/ui/opengl/render/ents.h"
#include "client/ui/opengl/render/hud.h"
#include "client/ui/opengl/render/selection.h"
#include "client/ui/opengl/render/shadows.h"
#include "client/ui/opengl/render/sun.h"
#include "client/ui/opengl/render/water.h"
#include "shared/opengl/loaders/obj.h"
#include "shared/opengl/render/text.h"
#include "shared/util/log.h"

#ifdef CRTS_PATHFINDING
#include "client/ui/opengl/render/pathfinding_overlay.h"
#endif

static struct hdarr *chunk_meshes;
static struct shadow_map shadow_map;
static struct water_fx wfx = {
	.reflect_w = 1024, .reflect_h = 512,
	.refract_w = 1024, .refract_h = 512,
};
struct camera reflect_cam;

bool
opengl_ui_render_setup(struct opengl_ui_ctx *ctx)
{
	obj_loader_setup();
	glEnable(GL_CLIP_DISTANCE0);
	ctx->clip_plane = 0;

	cam.pitch = ctx->opts.cam_pitch_max;
	cam.yaw   = ctx->opts.cam_yaw;

	sun.width = sun.height = ctx->opts.shadow_map_res;
	if (ctx->opts.shadows) {
		shadow_map.dim = ctx->opts.shadow_map_res;
		render_world_setup_shadows(&shadow_map);
	}

	reflect_cam.width = wfx.reflect_w;
	reflect_cam.height = wfx.reflect_h;
	if (ctx->opts.water) {
		render_world_setup_water(&wfx);
	}

#ifdef CRTS_PATHFINDING
	if (!render_world_setup_pathfinding_overlay()) {
		return false;
	}
#endif

	return render_world_setup_ents()
	       && render_world_setup_chunks(&chunk_meshes)
	       && render_world_setup_selection()
	       && render_world_setup_sun()
	       && render_text_setup(ctx->opts.font_scale);
}

void
opengl_ui_render_teardown(void)
{
	if (chunk_meshes) {
		hdarr_destroy(chunk_meshes);
	}
}

static void
render_everything(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	render_ents(hf, ctx);

	if (ctx->pass == rp_final) {
		render_selection(hf, ctx, chunk_meshes);
	}

	render_chunks(hf, ctx, chunk_meshes);

	if (ctx->pass == rp_final) {
		render_sun(ctx);
	}
}

static void
render_setup_frame(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	if (ctx->opts.water) {
		render_water_setup_frame(ctx);
	}

	render_ents_setup_frame(hf, ctx);

	render_selection_setup_frame(hf, ctx, chunk_meshes);

	render_chunks_setup_frame(hf, ctx, chunk_meshes);

	render_sun_setup_frame(ctx);

#ifdef CRTS_PATHFINDING
	if (hf->debug_path.on) {
		render_pathfinding_overlay_setup_frame(hf, ctx);
	}
#endif
}

static void
adjust_cameras(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	float w, h;
	static struct rectangle oref = { 0 };
	static float old_height;

	cam.width = ctx->win.width;
	cam.height = ctx->win.height;

	if (!cam.unlocked) {
		float a, b,
		/* boost the fov used for calculations to give us some padding */
		      fov = cam.fov * 0.6;

		a = cam.pos[1] * tanf(((PI / 2) - cam.pitch) + fov);
		b = cam.pos[1] * tanf(((PI / 2) - cam.pitch) - fov);

		h = a - b;
		/* TODO: the h calculation is precise but the w
		 * calculation is just a guess */
		w = h * (float)ctx->win.width / (float)ctx->win.height;

		ctx->ref.width = w;
		ctx->ref.height = h;

		if (cam.changed) {
			ctx->ref.pos.x = cam.pos[0] - w * 0.5;

			if (old_height != cam.pos[1]) {
				/* zooming, keep camera centered */
				ctx->ref.pos.y = cam.pos[2] - a;
			} else {
				/* tilting, move camera back */
				cam.pos[2] = ctx->ref.pos.y + a;
			}

			hf->cursor.x -= ctx->ref.pos.x - hf->view.x;
			hf->cursor.y -= ctx->ref.pos.y - hf->view.y;

			hf->view = ctx->ref.pos;
		} else {
			ctx->ref.pos = hf->view;

			cam.pos[0] = ctx->ref.pos.x + w * 0.5;
			cam.pos[2] = ctx->ref.pos.y + a;
		}

		sun.fov = ((cam.pos[1] / ctx->opts.cam_height_max) * 1.2) + 0.5;
		sun.changed = true;
	}

	/* update reflect cam */
	reflect_cam = cam;
	reflect_cam.pos[1] = cam.pos[1] * -1;
	reflect_cam.pitch = -cam.pitch;

	cam_calc_tgt(&cam);
	cam_calc_tgt(&reflect_cam);

	reflect_cam.changed = cam.changed = true;

	if ((ctx->ref_changed = memcmp(&oref, &ctx->ref, sizeof(struct rectangle)))) {
		oref = ctx->ref;
	}

	old_height = cam.pos[1];

	constrain_cursor(ctx, &hf->cursor);
}

static void
render_depth(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	ctx->pass = rp_depth;

	glViewport(0, 0, shadow_map.dim, shadow_map.dim);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_map.depth_map_fb);
	if (ctx->time.night) {
		glClearDepth(0.0);
		glClear(GL_DEPTH_BUFFER_BIT);
		glClearDepth(1.0);
	} else {
		glClear(GL_DEPTH_BUFFER_BIT);

		glCullFace(GL_FRONT);

		render_everything(ctx, hf);

		glCullFace(GL_BACK);
	}
}

static void
render_water_textures(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	struct camera tmpcam;

	/* water pass */
	ctx->pass = rp_final;

	/* reflections*/
	glViewport(0, 0, wfx.reflect_w, wfx.reflect_h);
	glBindFramebuffer(GL_FRAMEBUFFER, wfx.reflect_fb);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	ctx->clip_plane = 1;

	tmpcam = cam;
	cam = reflect_cam;

	render_everything(ctx, hf);

	ctx->clip_plane = 0;
	cam = tmpcam;

	/* refractions */
	glViewport(0, 0, wfx.refract_w, wfx.refract_h);
	glBindFramebuffer(GL_FRAMEBUFFER, wfx.refract_fb);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	ctx->clip_plane = 2;

	render_everything(ctx, hf);

	ctx->clip_plane = 0;
}

static void
position_sun(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	float sun_dist = 200;
	float sun_tilt = 70;

	float cx = ctx->ref.pos.x + ctx->ref.width / 2;
	float cy = ctx->ref.pos.y + ctx->ref.height / 2;

	sun.pos[0] = cx + sun_dist * sin(ctx->time.sun_theta);
	sun.pos[1] = sun_dist * cos(ctx->time.sun_theta);
	sun.pos[2] = cy + sun_tilt * sin(ctx->time.sun_theta);

	if ((ctx->time.night = sun.pos[1] < 0.0)) {
		sun.yaw = 0.0;
		sun.pitch = 0.0;
	} else {
		sun.yaw = tan(sun_tilt / sun_dist);
		sun.pitch = PI * 0.5 - ctx->time.sun_theta;
	}

	cam_calc_tgt(&sun);
}

static void
render_world(struct opengl_ui_ctx *ctx, struct hiface *hf)
{
	if (cam.changed || ctx->win.resized || !points_equal(&hf->view, &ctx->ref.pos)) {
		adjust_cameras(ctx, hf);
	}

	if (cam.changed || sun.changed || ctx->time.sun_theta != ctx->time.sun_theta_tgt) {
		if (fabs(ctx->time.sun_theta - ctx->time.sun_theta_tgt) < 0.001) {
			ctx->time.sun_theta = ctx->time.sun_theta_tgt;
		} else {
			ctx->time.sun_theta += (ctx->time.sun_theta_tgt - ctx->time.sun_theta) / 10;
		}

		position_sun(ctx, hf);
	}

	render_setup_frame(ctx, hf);

	/* shadows */
	if (ctx->opts.shadows) {
		render_depth(ctx, hf);
	}

	/* bind shadow texture */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadow_map.depth_map_tex);

	/* water textures */
	if (ctx->opts.water) {
		render_water_textures(ctx, hf);
	}

	/* final pass */
	{
		ctx->pass = rp_final;

		glViewport(0, 0, ctx->win.width, ctx->win.height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ctx->clip_plane = 1;
		render_everything(ctx, hf);
		ctx->clip_plane = 0;
	}

	/* water surface */
	if (ctx->opts.water) {
		render_water(ctx, &wfx);
	}

#ifdef CRTS_PATHFINDING
	if (hf->debug_path.on) {
		render_pathfinding_overlay(hf, ctx);
	}
#endif

	/* last usage of cam.changed */
	reflect_cam.changed = sun.changed = cam.changed = ctx->ref_changed = false;
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

	{
		render_text_clear();
		render_hud(ctx, hf);

		if (ctx->debug_hud) {
			render_debug_hud(ctx, hf);
		}

		render_text_commit();
		render_text(&ctx->win);
	}

	ctx->prof.setup = glfwGetTime() - start;

	glfwSwapBuffers(ctx->window);

	stop = glfwGetTime();
	ctx->prof.render = stop - ctx->prof.setup - start;
	ctx->prof.ftime = stop - start;
	last_start = start;

	ctx->win.resized = false;

	ctx->prof.smo_vert_count  = 0;
	ctx->prof.chunk_count = 0;
}
