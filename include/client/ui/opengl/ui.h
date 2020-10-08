#ifndef CLIENT_UI_OPENGL_UI_H
#define CLIENT_UI_OPENGL_UI_H
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "client/cfg/opengl.h"
#include "client/hiface.h"
#include "shared/math/linalg.h"
#include "shared/opengl/shader.h"
#include "shared/opengl/window.h"
#include "shared/pathfind/pgraph.h"

enum mouse_buttons {
	mb_1 = 1 << 0,
	mb_2 = 1 << 1,
	mb_3 = 1 << 2,
	mb_4 = 1 << 3,
	mb_5 = 1 << 4,
	mb_6 = 1 << 5,
	mb_7 = 1 << 6,
	mb_8 = 1 << 7,
};

enum modifier_types {
	mod_shift = 1 << 0,
	mod_ctrl  = 1 << 1,
};

struct opengl_ui_ctx {
	struct opengl_opts opts;
	struct rectangle ref;
	struct gl_win win;
	float pulse;
	GLFWwindow* window;
	struct {
		double lx, ly, x, y, dx, dy, scroll;
		double cursx, cursy;
		bool still, init;
		uint8_t buttons, old_buttons;
	} mouse;
	struct {
		uint8_t held[0xff];
		uint8_t mod;
		bool flying;
	} keyboard;
	struct {
		double ftime, setup, render;

		uint64_t smo_vert_count, chunk_count;
	} prof;

	char last_key;
	enum input_mode oim;
	struct keymap *ckm, *okm;

	bool reset_chunks, ref_changed;

	enum render_pass pass;

	bool debug_hud;

	struct {
		int32_t pitch;
	} cam_animation;

	struct {
		float sun_theta;
		float sun_theta_tgt;
		bool night;
	} time;

	uint32_t clip_plane;
};

struct opengl_ui_ctx *opengl_ui_init(void);
void opengl_ui_render(struct opengl_ui_ctx *nc, struct hiface *hf);
void opengl_ui_handle_input(struct opengl_ui_ctx *ctx, struct keymap **km,
	struct hiface *hf);
struct rectangle opengl_ui_viewport(struct opengl_ui_ctx *nc);
void opengl_ui_deinit(struct opengl_ui_ctx *ctx);
#endif
