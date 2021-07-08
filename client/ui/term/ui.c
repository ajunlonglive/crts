#include "posix.h"

#include <assert.h>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client/client.h"
#include "client/input_handler.h"
#include "client/ui/term/graphics.h"
#include "client/ui/term/graphics_cfg.h"
#include "client/ui/term/info.h"
#include "client/ui/term/ui.h"
#include "client/ui/term/world.h"
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

	clr = graphics.color_i++;
	term_setup_color_pair(fg, bg, clr);

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
	cat[type].pix.attr = term_attr_transform(attr);
	cat[type].pix.fg = fg;
	cat[type].pix.bg = bg;
	cat[type].zi = zi;

	return true;
}

bool
term_ui_init(struct term_ui_ctx *uic)
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

	uint32_t main = term_win_create_root(0.9, term_win_split_horizontal);
	uic->wins.world = term_win_create(main, 1.0, term_win_split_horizontal);

	uint32_t info = term_win_create(main, 0.5, term_win_split_vertical);
	uic->wins.info_l = term_win_create(info, 1.0, term_win_split_horizontal);
	uic->wins.info_r = term_win_create(info, 1.0, term_win_split_horizontal);

	term_commit_layout();

	return true;

fail_exit1:
	term_ui_deinit();

fail_exit2:
	if (redirected_log) {
		log_set_file(stderr);
		LOG_W(log_misc, "ncurses ui failing, see " DEF_LOGPATH " for more information");
	}

	return false;
}

void
term_ui_deinit(void)
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
term_ui_handle_input(struct term_ui_ctx *ctx, struct client *cli)
{
	term_poll_events(cli, key_cb, mouse_cb);
}

const struct rectangle *
term_ui_viewport(struct term_ui_ctx *nc)
{
	return term_win_rect(nc->wins.world);
}

void
term_ui_render(struct term_ui_ctx *nc, struct client *cli)
{
	TracyCZoneAutoS;

	term_check_resize();

	cli->redrew_world = draw_world(nc->wins.world, cli);

	term_clear_attr();

	draw_infol(nc->wins.info_l, cli);
	draw_infor(nc->wins.info_r, cli);

	TracyCZoneAutoE;
}
