#ifndef CLIENT_UI_GL_UI_H
#define CLIENT_UI_GL_UI_H
#include <glad/gl.h>

#include "client/cfg/opengl.h" // TODO
#include "shared/types/darr.h"
#include "shared/types/geom.h"
#include "shared/ui/gl/menu.h"
#include "shared/ui/gl/shader.h"
#include "shared/ui/gl/window.h"
#include "shared/util/timer.h"

#define MAX_GL_MAPS 64
#define INPUT_KEY_BUF_MAX 8

struct gl_ui_ctx {
	/* rendering */
	struct gl_win *win;
	struct { float x, y; } ref_pos;
	struct rectangle ref;
	bool reset_chunks, ref_changed;

	uint32_t clip_plane;

	enum render_pass pass;

	struct { int32_t pitch; } cam_animation;

	struct {
		float sun_theta;
		float sun_theta_tgt;
		bool night;
	} time;

	/* input */
	bool cursor_enabled;

	/* stats */
	struct {
		uint32_t friendly_ent_count;
		uint32_t live_ent_count;
	} stats;

	/* misc */
	struct opengl_opts opts; // TODO
	float pulse;
	struct timer timer;
	bool view_was_initialized;

	/* debugging stuff */
	struct {
		struct timer_avg setup, render;

		uint64_t smo_vert_count, chunk_count;
	} prof;

	bool debug_hud;
	uint32_t rendering_disabled;

#ifndef NDEBUG
	struct darr debug_hl_points;
#endif

	/* client */
	struct client *cli;

	/* menu */
	struct menu_ctx menu;
};

bool gl_ui_init(struct gl_ui_ctx *ctx);
void gl_ui_reset(struct gl_ui_ctx *ctx);
void gl_ui_render(struct gl_ui_ctx *ctx, struct client *cli);
const struct rectangle *gl_ui_viewport(struct gl_ui_ctx *nc);
void gl_ui_deinit(struct gl_ui_ctx *ctx);
#endif
