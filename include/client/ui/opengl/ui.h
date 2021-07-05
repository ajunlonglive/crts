#ifndef CLIENT_UI_OPENGL_UI_H
#define CLIENT_UI_OPENGL_UI_H
#include <glad/gl.h>

#include "client/cfg/opengl.h"
#include "client/client.h"
#include "client/ui/opengl/input_types.h"
#include "shared/math/linalg.h"
#include "shared/opengl/menu.h"
#include "shared/opengl/shader.h"
#include "shared/opengl/window.h"
#include "shared/util/timer.h"

#define MAX_OPENGL_MAPS 64
#define INPUT_KEY_BUF_MAX 8

struct opengl_ui_ctx {
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
	char last_key;
	enum input_mode oim;
	struct keymap **km, *ckm, *okm;

	struct {
		struct opengl_mouse_map mouse[MAX_OPENGL_MAPS];
		struct opengl_key_map keyboard[MAX_OPENGL_MAPS];
		uint8_t mouse_len;
		uint8_t keyboard_len;
	} input_maps[opengl_input_mode_count];
	enum opengl_input_mode im_mouse, im_mouse_new, im_keyboard, im_keyboard_new;

	/* stats */
	struct {
		uint32_t friendly_ent_count;
		uint32_t live_ent_count;
	} stats;

	/* misc */
	struct opengl_opts opts;
	float pulse;
	struct timer timer;

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

bool opengl_ui_init(struct opengl_ui_ctx *ctx);
void opengl_ui_render(struct opengl_ui_ctx *ctx, struct client *cli);
void opengl_ui_handle_input(struct opengl_ui_ctx *ctx, struct client *cli);
struct rectangle opengl_ui_viewport(struct opengl_ui_ctx *nc);
void opengl_ui_deinit(struct opengl_ui_ctx *ctx);
#endif
