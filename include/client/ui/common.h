#ifndef CLIENT_UI_COMMON_H
#define CLIENT_UI_COMMON_H

#include "client/client.h"
#include "client/input/cmdline.h"
#include "client/input/keymap.h"
#include "client/opts.h"

#ifdef NCURSES_UI
#include "client/ui/ncurses/ui.h"
#endif

#ifdef OPENGL_UI
#include "client/ui/opengl/ui.h"
#endif

enum ui_types {
	ui_null    = 0,
	ui_ncurses = 1 << 0,
	ui_opengl  = 1 << 1,
	ui_default = 1 << 7,
};

struct ui_ctx {
#ifdef NCURSES_UI
	struct ncurses_ui_ctx ncurses;
#endif
#ifdef OPENGL_UI
	struct opengl_ui_ctx opengl;
#endif
	uint8_t enabled;
};

void ui_init(struct c_opts *opts, struct ui_ctx *ctx);
void ui_render(struct ui_ctx *nc, struct client *cli);
void ui_handle_input(struct ui_ctx *ctx, struct keymap **km, struct client *cli);
struct rectangle ui_viewport(struct ui_ctx *nc);
void ui_deinit(struct ui_ctx *ctx);
enum cmd_result ui_cmdline_hook(struct cmd_ctx *cmd, struct ui_ctx *ctx, struct
	client *cli);
enum keymap_hook_result ui_keymap_hook(struct ui_ctx *ctx, struct keymap *km,
	char *err, const char *sec, const char *k, const char *v, uint32_t line);
#endif
