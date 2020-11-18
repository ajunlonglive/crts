#ifndef CLIENT_UI_OPENGL_UI_H
#define CLIENT_UI_OPENGL_UI_H
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "client/cfg/opengl.h"
#include "client/hiface.h"
#include "client/ui/opengl/input_types.h"
#include "shared/math/linalg.h"
#include "shared/opengl/shader.h"
#include "shared/opengl/window.h"

#define MAX_OPENGL_MAPS 64
#define INPUT_KEY_BUF_MAX 8

struct opengl_ui_ctx {
	/* rendering */
	struct rectangle ref;
	struct gl_win win;
	GLFWwindow* window;
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
	struct {
		double lx, ly, x, y, dx, dy, scroll,
		       scaled_dx, scaled_dy;
		double cursx, cursy;
		bool still, init;
		uint8_t buttons, old_buttons;
	} mouse;

	struct {
		uint8_t mod;
		uint16_t held[INPUT_KEY_BUF_MAX];
		uint8_t held_len;
		bool flying;
	} keyboard;

	char last_key;
	enum input_mode oim;
	struct keymap **km, *ckm, *okm;

	struct {
		struct opengl_mouse_map mouse[MAX_OPENGL_MAPS];
		struct opengl_key_map keyboard[MAX_OPENGL_MAPS];
		uint8_t mouse_len;
		uint8_t keyboard_len;
	} input_maps[opengl_input_mode_count];
	enum opengl_input_mode im;

	/* misc */
	struct opengl_opts opts;
	float pulse;

	/* debugging stuff */
	struct {
		double ftime, setup, render;

		uint64_t smo_vert_count, chunk_count;
	} prof;

	bool debug_hud;

	struct darr debug_hl_points;

	/* hiface */
	struct hiface *hf;
};

bool opengl_ui_init(struct opengl_ui_ctx *ctx);
void opengl_ui_render(struct opengl_ui_ctx *nc, struct hiface *hf);
void opengl_ui_handle_input(struct opengl_ui_ctx *ctx, struct keymap **km,
	struct hiface *hf);
struct rectangle opengl_ui_viewport(struct opengl_ui_ctx *nc);
void opengl_ui_deinit(struct opengl_ui_ctx *ctx);
#endif
