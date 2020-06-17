#include "posix.h"

#include <assert.h>
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>

#include "client/cfg/common.h"
#include "client/cfg/graphics.h"
#include "client/hiface.h"
#include "client/input/handler.h"
#include "client/input/mouse.h"
#include "client/ui/ncurses/container.h"
#include "client/ui/ncurses/info.h"
#include "client/ui/ncurses/ui.h"
#include "client/ui/ncurses/world.h"
#include "shared/util/log.h"

struct ncurses_ui_ctx {
	struct display_container dc;
};

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

struct ncurses_ui_ctx *
ncurses_ui_init(char *logpath)
{
	if (!isatty(STDOUT_FILENO)) {
		LOG_W("stdout is not a tty");
		return NULL;
	}

	struct ncurses_ui_ctx *uic = calloc(1, sizeof(struct ncurses_ui_ctx));
	struct parse_graphics_ctx cfg_ctx = { NULL, ncurses_color_setup };

	FILE *logfile;
	if ((logfile = fopen(logpath, "w"))) {
		L("redirecting logs to %s", logpath);
		logfiled = fileno(logfile);
	} else {
		L("failed to redirect %s", logpath);
	}
	term_setup();

	if (!parse_cfg_file(GRAPHICS_CFG, &cfg_ctx, parse_graphics_handler)) {
		goto fail_exit;
	}

	init_tile_curs();

	dc_init(&uic->dc);

	return uic;

fail_exit:
	ncurses_ui_deinit();
	return NULL;
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
	default:
		return k;
	}
}

static uint64_t
transform_bstate(uint64_t bstate)
{
	uint64_t nbs = 0;

	if (bstate & BUTTON1_PRESSED) {
		nbs |= ms_b1_press;
	}
	if (bstate & BUTTON1_RELEASED) {
		nbs |= ms_b1_release;
	}
	if (bstate & BUTTON3_PRESSED) {
		nbs |= ms_b3_press;
	}
	if (bstate & BUTTON3_RELEASED) {
		nbs |= ms_b3_release;
	}

	return nbs;
}

void
ncurses_ui_handle_input(struct keymap **km, struct hiface *hf)
{
	int key;
	MEVENT event;

	while ((key = getch()) != ERR) {
		if (key == KEY_MOUSE) {
			getmouse(&event);
			handle_mouse(event.x, event.y, transform_bstate(event.bstate), hf);
		} else if ((*km = handle_input(*km, transform_key(key), hf)) == NULL) {
			*km = &hf->km[hf->im];
		}
	}
}

struct rectangle
ncurses_ui_viewport(struct ncurses_ui_ctx *nc)
{
	return nc->dc.root.world->rect;
}

static void
fix_cursor(const struct rectangle *r, struct point *vu, struct point *cursor)
{
	int diff;

	if (point_in_rect(cursor, r)) {
		return;
	}

	if ((diff = 0 - cursor->y) > 0 || (diff = (r->height - 1) - cursor->y) < 0) {
		vu->y -= diff;
		cursor->y += diff;
	}

	if ((diff = 0 - cursor->x) > 0 || (diff = (r->width - 1) - cursor->x) < 0) {
		vu->x -= diff;
		cursor->x += diff;
	}
}

void
ncurses_ui_render(struct ncurses_ui_ctx *nc, struct hiface *hf)
{
	term_check_resize();

	if (hf->im == im_select) {
		fix_cursor(&nc->dc.root.world->rect, &hf->view, &hf->cursor);
	}

	if (hf->center_cursor) {
		hf->view.x += hf->cursor.x - nc->dc.root.world->rect.width / 2;
		hf->view.y += hf->cursor.y - nc->dc.root.world->rect.height / 2;
		hf->cursor.x = nc->dc.root.world->rect.width / 2;
		hf->cursor.y = nc->dc.root.world->rect.height / 2;

		/* TODO: add center lock? */
		hf->center_cursor = false;
	}

	hf->redrew_world = draw_world(nc->dc.root.world, hf);

	win_clr_attr();

	draw_infol(nc->dc.root.info.l, hf);

	draw_infor(nc->dc.root.info.r, hf);
}
