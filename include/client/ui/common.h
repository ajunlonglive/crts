#ifndef CLIENT_UI_COMMON_H
#define CLIENT_UI_COMMON_H

#include "client/cmdline.h"
#include "client/opts.h"
#include "shared/math/linalg.h"

#ifdef NCURSES_UI
#include "client/ui/term/ui.h"
#endif

#ifdef OPENGL_UI
#include "client/ui/gl/ui.h"
#endif

enum ui_types {
	ui_default = 0,
	ui_null    = 1 << 0,
	ui_term    = 1 << 1,
	ui_gl      = 1 << 2,
};

struct ui_ctx {
#ifdef NCURSES_UI
	struct term_ui_ctx term;
#endif
#ifdef OPENGL_UI
	struct gl_ui_ctx gl;
#endif
	uint8_t enabled;
};

struct client;
struct cmd_ctx;

void ui_init(struct client_opts *opts, struct ui_ctx *ctx);
void ui_reset(struct client *cli);
void ui_render(struct client *cli);
void ui_handle_input(struct client *cli);
void ui_deinit(struct ui_ctx *ctx);
enum cmd_result ui_cmdline_hook(struct cmd_ctx *cmd, struct client *cli);
vec3 *ui_cam_pos(struct client *cli);
struct rect *ui_ref(struct client *cli);
#endif
