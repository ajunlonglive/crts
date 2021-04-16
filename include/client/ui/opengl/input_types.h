#ifndef CLIENT_UI_OPENGL_INPUT_TYPES_H
#define CLIENT_UI_OPENGL_INPUT_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#include "client/input/keymap.h"

enum mouse_buttons {
	mb_no = 0,
	mb_1 = 1 << 1,
	mb_2 = 1 << 2,
	mb_3 = 1 << 3,
	mb_4 = 1 << 4,
	mb_5 = 1 << 5,
	mb_6 = 1 << 6,
	mb_7 = 1 << 7,
	mb_8 = 1 << 8,
};

enum modifier_types {
	mod_shift = 1 << 0,
	mod_ctrl  = 1 << 1,
};

enum mouse_map_type {
	mmt_click,
	mmt_drag,
	mmt_scroll,
};

enum opengl_input_mode {
	oim_normal,
	oim_flying,
	oim_released,
	opengl_input_mode_count
};

enum mouse_action_scroll {
	mas_noop,
	mas_zoom,
	mas_quick_zoom,
};

enum mouse_action_drag {
	mad_noop,
	mad_move_view,
	mad_move_cursor_neutral,
	mad_move_cursor_create,
	mad_move_cursor_destroy,
	mad_point_camera,
};

struct opengl_mouse_map {
	enum modifier_types mod;
	enum mouse_buttons button;
	enum mouse_map_type type;
	union {
		struct {
			uint32_t kc;
			bool is_opengl_kc;
		} click;
		enum mouse_action_scroll scroll;
		enum mouse_action_drag drag;
	} action;
	bool active;
};

enum opengl_key_command {
	okc_toggle_wireframe,
	okc_toggle_camera_lock,
	okc_toggle_debug_hud,
	okc_toggle_look_angle,
	okc_release_mouse,
	okc_capture_mouse,
	okc_fly_forward,
	okc_fly_left,
	okc_fly_right,
	okc_fly_back,
	okc_toggle_render_step_ents, // make sure this is the first "toggle_render_step"
	okc_toggle_render_step_selection,
	okc_toggle_render_step_chunks,
	okc_toggle_render_step_shadows,
	okc_toggle_render_step_reflections,
	opengl_key_command_count
};

struct opengl_key_map {
	enum modifier_types mod;
	uint16_t key;
	enum opengl_key_command action;
};

#endif
