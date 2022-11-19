#include "posix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef OPENGL_UI
#define OPENGL_UI
#endif

#include "client/input_handler.h"
#include "client/opts.h"
#include "client/ui/gl/globals.h"
#include "client/ui/gl/input.h"
#include "client/ui/gl/render.h"
#include "client/ui/gl/render/chunks.h"
#include "client/ui/gl/render/ents.h"
#include "client/ui/gl/render/final_quad.h"
#include "client/ui/gl/render/hud.h"
#include "client/ui/gl/render/selection.h"
#include "client/ui/gl/render/shadows.h"
#include "client/ui/gl/render/sun.h"
#include "client/ui/gl/render/water.h"
#include "shared/ui/gl/loaders/obj.h"
#include "shared/ui/gl/menu.h"
#include "shared/util/log.h"
#include "tracy.h"

#ifndef NDEBUG
#include "client/ui/gl/render/pathfinding_overlay.h"
#define RENDER_STEP(disabled, step) (!(disabled & step))
#else
#define RENDER_STEP(disabled, step) true
#endif

static struct shadow_map shadow_map;
static struct water_fx wfx = {
	.reflect_w = 512, .reflect_h = 256,
	.refract_w = 512, .refract_h = 256,
};
static struct final_quad_fx fqfx;
struct camera reflect_cam;

bool
gl_ui_render_setup(struct gl_ui_ctx *ctx)
{
	obj_loader_setup();
	glEnable(GL_CLIP_DISTANCE0);
	ctx->clip_plane = 0;

	const float shadow_map_res = 4096;
	sun.width = sun.height = shadow_map_res;
	shadow_map.dim = shadow_map_res;
	render_world_setup_shadows(&shadow_map);

	reflect_cam.width = wfx.reflect_w;
	reflect_cam.height = wfx.reflect_h;
	render_world_setup_water(&wfx);

	render_world_setup_final_quad(ctx, &fqfx);

#ifndef NDEBUG
	if (!render_world_setup_pathfinding_overlay()) {
		return false;
	}
#endif

	if (!menu_setup()) {
		return false;
	}

	return render_world_setup_ents()
	       && render_world_setup_chunks(&ctx->chunk_meshes)
	       && render_world_setup_selection()
	       && render_world_setup_sun();
}

void
gl_ui_render_teardown(void)
{
}

static void
render_everything(struct gl_ui_ctx *ctx, struct client *cli, bool render_chunk_bottoms)
{
	TracyCZoneAutoS;

	if (RENDER_STEP(ctx->rendering_disabled, gl_render_step_chunks)) {
		render_chunks(cli, ctx, &ctx->chunk_meshes, render_chunk_bottoms);
	}

	if (RENDER_STEP(ctx->rendering_disabled, gl_render_step_ents)) {
		render_ents(cli, ctx);
	}

	if (RENDER_STEP(ctx->rendering_disabled, gl_render_step_selection)
	    && ctx->pass == rp_final) {
		render_selection(cli, ctx);
	}

	if (ctx->pass == rp_final) {
		render_sun(ctx);
	}

	TracyCZoneAutoE;
}

static void
render_setup_frame(struct gl_ui_ctx *ctx, struct client *cli)
{
	TracyCZoneAutoS;

	if (RENDER_STEP(ctx->rendering_disabled, gl_render_step_reflections)) {
		render_water_setup_frame(cli, ctx);
	}

	if (RENDER_STEP(ctx->rendering_disabled, gl_render_step_ents)) {
		render_ents_setup_frame(cli, ctx);
	}

	if (RENDER_STEP(ctx->rendering_disabled, gl_render_step_selection)) {
		render_selection_setup_frame(cli, ctx);
	}

	if (RENDER_STEP(ctx->rendering_disabled, gl_render_step_chunks)) {
		render_chunks_setup_frame(cli, ctx, &ctx->chunk_meshes);
	}

	render_sun_setup_frame(ctx);
	render_final_quad_setup_frame(ctx, &fqfx);

#ifndef NDEBUG
	if (cli->debug_path.on) {
		render_pathfinding_overlay_setup_frame(cli, ctx);
	}
#endif

	TracyCZoneAutoE;
}

static void
render_depth(struct gl_ui_ctx *ctx, struct client *cli)
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

		render_everything(ctx, cli, false);
	}
}

static void
render_water_textures(struct gl_ui_ctx *ctx, struct client *cli)
{
	struct camera tmpcam;

	/* water pass */
	ctx->pass = rp_final;

	/* reflections*/
	glViewport(0, 0, wfx.reflect_w, wfx.reflect_h);
	glBindFramebuffer(GL_FRAMEBUFFER, wfx.reflect_fb);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	tmpcam = cam;
	cam = reflect_cam;

	ctx->clip_plane = 1;
	render_everything(ctx, cli, true);
	ctx->clip_plane = 0;

	cam = tmpcam;

	/* refractions */
	glViewport(0, 0, wfx.refract_w, wfx.refract_h);
	glBindFramebuffer(GL_FRAMEBUFFER, wfx.refract_fb);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	ctx->clip_plane = 2;
	render_everything(ctx, cli, false);
	ctx->clip_plane = 0;
}

static void
position_sun(struct gl_ui_ctx *ctx, struct client *cli)
{
	float sun_dist = 200;
	float sun_tilt = 70;

	sun.pos[0] = cli->ref.center.x + sun_dist * sin(ctx->time.sun_theta);
	sun.pos[1] = sun_dist * cos(ctx->time.sun_theta);
	sun.pos[2] = cli->ref.center.y + sun_tilt * sin(ctx->time.sun_theta);

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
render_world(struct gl_ui_ctx *ctx, struct client *cli)
{
	{ // adjust cameras
		float w, h;
		static struct rect oref = { 0 };
		/* static float old_height; */

		if (!ctx->view_was_initialized && (cli->state & csf_view_initialized)) {
			/* L(log_cli, "copying initial ref from cli"); */
			/* cli->ref = cli->ref; */
			/* cam.pos[0] = ctx->real_ref[0].x; */
			/* cam.pos[2] = ctx->real_ref[0].y; */
			ctx->view_was_initialized = true;
		}

		cam.width = ctx->win->sc_width;
		cam.height = ctx->win->sc_height;

		if (!cam.unlocked) {
			float a, b,
			/* boost the fov used for calculations to give us some padding */
			      fov = cam.fov * 0.6;

			a = cam.pos[1] * tanf(((PI / 2) - cam.pitch) + fov);
			b = cam.pos[1] * tanf(((PI / 2) - cam.pitch) - fov);

			h = a - b;
			/* TODO: the h calculation is precise but the w
			 * calculation is just a guess */
			w = h * (float)ctx->win->sc_width / (float)ctx->win->sc_height;

			cli->ref.w = w;
			cli->ref.h = h;
			make_rotated_rect(&cli->ref.center, cli->ref.h, cli->ref.w, cli->ref.angle, &cli->ref.rect);

			struct pointf new_cam_pos;
			pointf_relative_to_rect(&cli->ref.rect, cli->ref.w / 2.0f, cli->ref.h - a, &new_cam_pos);
			cam.pos[0] = new_cam_pos.x;
			cam.pos[2] = new_cam_pos.y;
			cam.yaw = PI - cli->ref.angle;

			sun.changed = true;
		}

		/* update reflect cam */
		reflect_cam = cam;
		reflect_cam.pos[1] = cam.pos[1] * -1;
		reflect_cam.pitch = -cam.pitch;
		reflect_cam.yaw = cam.yaw;

		cam_calc_tgt(&cam);
		cam_calc_tgt(&reflect_cam);

		reflect_cam.changed = cam.changed = true;

		if ((ctx->ref_changed = memcmp(&oref, &cli->ref, sizeof(struct rect)))) {
			oref = cli->ref.rect;
		}
	}

	if (cam.changed || sun.changed || ctx->time.sun_theta != ctx->time.sun_theta_tgt) {
		if (fabs(ctx->time.sun_theta - ctx->time.sun_theta_tgt) < 0.001) {
			ctx->time.sun_theta = ctx->time.sun_theta_tgt;
		} else {
			ctx->time.sun_theta += (ctx->time.sun_theta_tgt - ctx->time.sun_theta) / 10;
		}

		position_sun(ctx, cli);
	}

	render_setup_frame(ctx, cli);

	/* shadows */
	if (RENDER_STEP(ctx->rendering_disabled, gl_render_step_shadows)) {
		render_depth(ctx, cli);
	}

	/* bind shadow texture */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadow_map.depth_map_tex);

	/* water textures */
	if (RENDER_STEP(ctx->rendering_disabled, gl_render_step_reflections)) {
		render_water_textures(ctx, cli);
	}

	/* final pass */
	{
		ctx->pass = rp_final;

		glViewport(0, 0, ctx->win->px_width, ctx->win->px_height);
		glBindFramebuffer(GL_FRAMEBUFFER, fqfx.fb);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ctx->clip_plane = 1;
		render_everything(ctx, cli, false);

		/* water surface */
		if (RENDER_STEP(ctx->rendering_disabled, gl_render_step_reflections)) {
			render_water(ctx, &wfx);
		}
		ctx->clip_plane = 0;

		glViewport(0, 0, ctx->win->px_width, ctx->win->px_height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render_final_quad(cli, ctx, &fqfx);
	}

#ifndef NDEBUG
	if (cli->debug_path.on) {
		render_pathfinding_overlay(cli, ctx);
	}
#endif

	/* last usage of cam.changed */
	reflect_cam.changed = sun.changed = cam.changed = ctx->ref_changed = false;
}

void
gl_ui_render(struct gl_ui_ctx *ctx, struct client *cli)
{
	if (!gl_win_is_focused()) {
		nanosleep(&(struct timespec){
			.tv_nsec = ((1.0f / 30.0f)) * 1000000000
		}, NULL);

		return;
	}

	TracyCZoneAutoS;

	timer_lap(&ctx->timer);
	ctx->pulse_ms = monotonic_ms();

	TracyCZoneN(tctx_render_setup, "rendering calls", true);

	render_world(ctx, cli);
	render_hud(ctx, cli);

	TracyCZoneEnd(tctx_render_setup);

	timer_avg_push(&ctx->prof.setup, timer_lap(&ctx->timer));

	TracyCZoneN(tctx_render_swap_buffers, "swap buffers", true);

	gl_win_swap_buffers();

	TracyCZoneEnd(tctx_render_swap_buffers);

	timer_avg_push(&ctx->prof.render, timer_lap(&ctx->timer));

	ctx->win->resized = false;

	ctx->prof.smo_vert_count  = 0;
	ctx->prof.chunk_count = 0;

	TracyCZoneAutoE;
}
