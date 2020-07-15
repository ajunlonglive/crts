#ifndef CLIENT_UI_COMMON_H
#define CLIENT_UI_COMMON_H

#include "client/hiface.h"
#include "client/input/cmdline.h"
#include "client/input/keymap.h"
#include "client/opts.h"

enum ui_types {
	ui_null    = 0,
	ui_ncurses = 1 << 0,
	ui_opengl  = 1 << 1,
	ui_default = 1 << 7,
};

struct ui_ctx {
	struct ncurses_ui_ctx *ncurses;
	struct opengl_ui_ctx *opengl;
	uint8_t enabled;
};


void ui_init(struct c_opts *opts, struct ui_ctx *ctx);
void ui_render(struct ui_ctx *nc, struct hiface *hf);
void ui_handle_input(struct ui_ctx *ctx, struct keymap **km, struct hiface *hf);
struct rectangle ui_viewport(struct ui_ctx *nc);
void ui_deinit(struct ui_ctx *ctx);
enum cmd_result ui_cmdline_hook(struct cmd_ctx *cmd, struct ui_ctx *ctx, struct hiface *hf);
#endif
