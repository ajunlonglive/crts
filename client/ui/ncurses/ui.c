#include "posix.h"

#include <assert.h>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client/client.h"
#include "client/input_handler.h"
#include "client/ui/ncurses/container.h"
#include "client/ui/ncurses/graphics_cfg.h"
#include "client/ui/ncurses/info.h"
#include "client/ui/ncurses/ui.h"
#include "client/ui/ncurses/world.h"
#include "shared/input/keyboard.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"
#include "tracy.h"

#define DEF_LOGPATH "debug.log"

static bool
ncurses_color_setup(void *_, int32_t sect, int32_t type,
	char c, short fg, short bg, short attr, short zi)
{
	uint32_t clr;
	struct graphics_info_t *cat = NULL;

	switch (sect) {
	case gfx_cfg_section_tiles:
		cat = graphics.tiles;
		break;
	case gfx_cfg_section_entities:
		cat = graphics.entities;
		break;
	case gfx_cfg_section_cursor:
		cat = graphics.cursor;
		break;
	case gfx_cfg_section_global:
		L(log_misc, "element must not be in global namespace");
		return false;
		break;
	}

	assert(cat != NULL);

	clr = setup_color_pair(&graphics, fg, bg);

	if (bg == TRANS_COLOR) {
		if (zi == 0) {
			L(log_misc, "zi must be > 0 for transparent bg");
			return false;
		}

		graphics.trans_bg.fg_map[fg + TRANS_COLOR_BUF] = graphics.trans_bg.fgi++;

		if (graphics.trans_bg.fgi >= TRANS_COLORS) {
			L(log_misc, "too many transparent backgrounds");
			return false;
		}
	}

	cat[type].pix.c = c;
	cat[type].pix.clr = clr;
	cat[type].pix.attr = attr_transform(attr);
	cat[type].pix.fg = fg;
	cat[type].pix.bg = bg;
	cat[type].zi = zi;

	return true;
}

bool
ncurses_ui_init(struct ncurses_ui_ctx *uic)
{
	setlocale(LC_ALL, "");

	bool redirected_log = false;

	if (!isatty(STDOUT_FILENO)) {
		LOG_W(log_misc, "stdout is not a tty");
		return NULL;
	}

	if (log_file_is_a_tty()) {
		L(log_misc, "attempting to redirect logs to " DEF_LOGPATH);

		FILE *f;
		if ((f = fopen(DEF_LOGPATH, "wb"))) {
			log_set_file(f);
			redirected_log = true;
		} else {
			LOG_W(log_misc, "failed to redirect logs to '%s': '%s'", DEF_LOGPATH, strerror(errno));
			goto fail_exit2;
		}
	}

	term_setup();

	struct parse_graphics_ctx cfg_ctx = { NULL, ncurses_color_setup };

	if (!parse_cfg_file("curses.ini", &cfg_ctx, parse_graphics_handler)) {
		LOG_W(log_misc, "failed to parse graphics");
		goto fail_exit1;
	}

	init_tile_curs();

	dc_init(&uic->dc);

	return true;

fail_exit1:
	ncurses_ui_deinit();

fail_exit2:
	if (redirected_log) {
		log_set_file(stderr);
		LOG_W(log_misc, "ncurses ui failing, see " DEF_LOGPATH " for more information");
	}

	return false;
}

void
ncurses_ui_deinit(void)
{
	term_teardown();
}

static void
key_cb(void *ctx, uint8_t key, uint8_t mod, enum key_action action)
{
	input_handle_key(ctx, key, mod, action);
}

static void
mouse_cb(void *ctx, float dx, float dy)
{
	input_handle_mouse(ctx, dx, dy);
}

void
ncurses_ui_handle_input(struct ncurses_ui_ctx *ctx, struct client *cli)
{
	term_win_poll_events(cli, key_cb, mouse_cb);
}

struct rectangle
ncurses_ui_viewport(struct ncurses_ui_ctx *nc)
{
	return nc->dc.root.world->rect;
}

void
ncurses_ui_render(struct ncurses_ui_ctx *nc, struct client *cli)
{
	TracyCZoneAutoS;

	term_check_resize();

	cli->redrew_world = draw_world(nc->dc.root.world, cli);

	win_clr_attr();

	draw_infol(nc->dc.root.info.l, cli);

	draw_infor(nc->dc.root.info.r, cli);

	TracyCZoneAutoE;
}
