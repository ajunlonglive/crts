#ifndef CLIENT_UI_COMMON_H
#define CLIENT_UI_COMMON_H

#include "client/hiface.h"
#include "client/input/keymap.h"
#include "client/opts.h"

enum ui_types {
	ui_null    = 0,
	ui_ncurses = 1 << 0,
	ui_opengl  = 1 << 1,
	ui_default = 1 << 7,
};

struct ui_ctx;

struct ui_ctx *ui_init(struct opts *opts);
void ui_render(struct ui_ctx *nc, struct hiface *hf);
void ui_handle_input(struct ui_ctx *ctx, struct keymap **km, struct hiface *hf);
struct rectangle ui_viewport(struct ui_ctx *nc);
void ui_deinit(struct ui_ctx *ctx);
#endif
