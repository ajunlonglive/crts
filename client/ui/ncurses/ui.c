#include "posix.h"

#include <assert.h>
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>

#include "client/cfg/graphics.h"
#include "client/hiface.h"
#include "client/input/handler.h"
#include "client/ui/ncurses/container.h"
#include "client/ui/ncurses/info.h"
#include "client/ui/ncurses/ui.h"
#include "client/ui/ncurses/world.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"

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
		L("element must not be in global namespace");
		return false;
		break;
	}

	assert(cat != NULL);

	clr = setup_color_pair(&graphics, fg, bg);

	if (bg == TRANS_COLOR) {
		if (zi == 0) {
			L("zi must be > 0 for transparent bg");
			return false;
		}

		graphics.trans_bg.fg_map[fg + TRANS_COLOR_BUF] = graphics.trans_bg.fgi++;

		if (graphics.trans_bg.fgi >= TRANS_COLORS) {
			L("too many transparent backgrounds");
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
	bool redirected_log;

	if (!isatty(STDOUT_FILENO)) {
		LOG_W("stdout is not a tty");
		return NULL;
	}

	struct parse_graphics_ctx cfg_ctx = { NULL, ncurses_color_setup };

	if (logfile == stderr) {
		set_log_file(DEF_LOGPATH);
		redirected_log = true;
	}

	term_setup();

	if (!parse_cfg_file(GRAPHICS_CFG, &cfg_ctx, parse_graphics_handler)) {
		goto fail_exit;
	}

	init_tile_curs();

	dc_init(&uic->dc);

	return true;

fail_exit:
	ncurses_ui_deinit();

	if (redirected_log) {
		logfile = stderr;
		LOG_W("ncurses ui failing, see " DEF_LOGPATH " for more information");
	}

	return false;
}

void
ncurses_ui_deinit(void)
{
	term_teardown();
}

static unsigned
transform_key(unsigned k)
{
	switch (k) {
	case KEY_UP:
		return skc_up;
	case KEY_DOWN:
		return skc_down;
	case KEY_LEFT:
		return skc_left;
	case KEY_RIGHT:
		return skc_right;
	case KEY_ENTER:
	case 13:
		return '\n';
	case KEY_BACKSPACE:
	case 127:
		return '\b';
	default:
		return k;
	}
}

void
ncurses_ui_handle_input(struct keymap **km, struct hiface *hf)
{
	int key;

	while ((key = getch()) != ERR) {
		if ((*km = handle_input(*km, transform_key(key), hf)) == NULL) {
			*km = &hf->km[hf->im];
		}
	}
}

struct rectangle
ncurses_ui_viewport(struct ncurses_ui_ctx *nc)
{
	return nc->dc.root.world->rect;
}

void
ncurses_ui_render(struct ncurses_ui_ctx *nc, struct hiface *hf)
{
	term_check_resize();

	hf->redrew_world = draw_world(nc->dc.root.world, hf);

	win_clr_attr();

	draw_infol(nc->dc.root.info.l, hf);

	draw_infor(nc->dc.root.info.r, hf);
}
