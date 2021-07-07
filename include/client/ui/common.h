#ifndef CLIENT_UI_COMMON_H
#define CLIENT_UI_COMMON_H

#include "client/client.h"
#include "client/opts.h"

#ifdef NCURSES_UI
#include "client/ui/ncurses/ui.h"
#endif

#ifdef OPENGL_UI
#include "client/ui/opengl/ui.h"
#endif

enum ui_types {
	ui_default = 0,
	ui_null    = 1 << 0,
	ui_ncurses = 1 << 1,
	ui_opengl  = 1 << 2,
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

void ui_init(struct client_opts *opts, struct ui_ctx *ctx);
void ui_render(struct client *cli);
void ui_handle_input(struct client *cli);
void ui_deinit(struct ui_ctx *ctx);
enum cmd_result ui_cmdline_hook(struct cmd_ctx *cmd, struct client *cli);
vec3 *ui_cam_pos(struct client *cli);
#endif
