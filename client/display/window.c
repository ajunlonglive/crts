#define _POSIX_C_SOURCE 199309L

#ifdef __APPLE__
// POSIX C source doesn't give us SIGWINCH on bsd
#define _DARWIN_C_SOURCE
#endif

#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "client/display/window.h"
#include "client/graphics.h"
#include "shared/math/geom.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

struct {
	struct darr *wins;
	bool resized;
} term;

static void
calc_proportion(struct rectangle *sub, const struct rectangle *par, enum win_split sp, double pct, bool primary)
{
	const int *dim, *coord;
	int *newdim, *newcoord;

	switch (sp) {
	case ws_vertical:
		dim = &par->width;
		newdim = &sub->width;
		coord = &par->pos.x;
		newcoord = &sub->pos.x;
		break;
	case ws_horizontal:
		dim = &par->height;
		newdim = &sub->height;
		coord = &par->pos.y;
		newcoord = &sub->pos.y;
		break;
	}

	int primary_size = pct * (double)(*dim);

	if (primary) {
		*newdim = primary_size;
	} else {
		*newdim = *dim - primary_size;
		*newcoord = *coord + primary_size;
	}
}

static enum iteration_result
resize_iterator(void *_, void *_win)
{
	struct win *win = _win, *parent = darr_get(term.wins, win->parent);

	win->rect = parent->rect;

	if (parent->children[1] == 0) {
		return ir_cont;
	}

	bool primary = win->index == 0 || parent->children[0] == win->index;

	calc_proportion(&win->rect, &parent->rect, parent->split,
		parent->split_pct, primary);

	return ir_cont;
}

static void
get_term_dimensions(int *height, int *width)
{
	struct winsize w;

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	*width = w.ws_col;
	*height = w.ws_row;
}

void
term_commit_layout(void)
{
	darr_for_each(term.wins, NULL, resize_iterator);
}

bool
term_check_resize(void)
{
	if (!term.resized) {
		return false;
	}

	struct win *root_win = darr_get(term.wins, 0);

	get_term_dimensions(&root_win->rect.height, &root_win->rect.width);

	resize_term(root_win->rect.height, root_win->rect.width);
	wresize(stdscr, root_win->rect.height, root_win->rect.width);

	L("terminal changed size: %dx%d", root_win->rect.height, root_win->rect.width);

	term_commit_layout();
	term.resized = false;
	return true;
}

static void
handle_sigwinch(int _)
{
	term.resized = true;
}

static void
install_signal_handler(void)
{
	struct sigaction sigact;

	memset(&sigact, 0, sizeof(struct sigaction));

	sigact.sa_flags = 0;
	sigact.sa_handler = handle_sigwinch;
	sigaction(SIGWINCH, &sigact, NULL);
}

static void
init_color_pairs(void)
{
	init_pair(color_blk, COLOR_BLACK, -1);
	init_pair(color_red, COLOR_RED, -1);
	init_pair(color_grn, COLOR_GREEN, -1);
	init_pair(color_ylw, COLOR_YELLOW, -1);
	init_pair(color_blu, COLOR_BLUE, -1);
	init_pair(color_mag, COLOR_MAGENTA, -1);
	init_pair(color_cyn, COLOR_CYAN, -1);
	init_pair(color_wte, COLOR_WHITE, -1);
	init_pair(color_bg_blk, -1, COLOR_BLACK);
	init_pair(color_bg_red, -1, COLOR_RED);
	init_pair(color_bg_grn, -1, COLOR_GREEN);
	init_pair(color_bg_ylw, -1, COLOR_YELLOW);
	init_pair(color_bg_blu, -1, COLOR_BLUE);
	init_pair(color_bg_mag, -1, COLOR_MAGENTA);
	init_pair(color_bg_cyn, -1, COLOR_CYAN);
	init_pair(color_bg_wte, -1, COLOR_WHITE);
}

static void
win_init(struct win *win)
{
	memset(win, 0, sizeof(struct win));

	win->split_pct = 1.0;
	win->split = ws_horizontal;
}

void
term_setup(void)
{
	struct win root_win;

	setenv("ESCDELAY", "10", 1);
	initscr();
	cbreak();
	noecho();
	nonl();
	start_color();
	use_default_colors();
	init_color_pairs();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	wbkgdset(stdscr, ' ');
	curs_set(0);

	win_init(&root_win);
	get_term_dimensions(&root_win.rect.height, &root_win.rect.width);

	memset(&term, 0, sizeof(term));
	term.wins = darr_init(sizeof(struct win));
	darr_push(term.wins, &root_win);

	L("setup root window");

	install_signal_handler();
}

void
term_teardown(void)
{
	darr_destroy(term.wins);
	endwin();
}

struct win *
win_create(struct win *parent)
{
	struct win win, *winp;
	size_t i;

	if (parent == NULL) {
		parent = darr_get(term.wins, 0);
	}

	win_init(&win);
	win.parent = parent->index;

	i = darr_push(term.wins, &win);
	winp = darr_get(term.wins, i);
	winp->index = i;

	if (parent->children[0] == 0) {
		parent->children[0] = i;
	} else {
		parent->children[1] = i;
	}

	return winp;
}

void
set_color(enum color c)
{
	attron(COLOR_PAIR(c));
}

void
unset_color(enum color c)
{
	attroff(COLOR_PAIR(c));
}

void
win_write(const struct win *win, const struct point *p, char c)
{
	struct point np = {
		.x = win->rect.pos.x + p->x,
		.y = win->rect.pos.y + p->y,
	};

	if (point_in_rect(&np, &win->rect)) {
		mvwaddch(stdscr, np.y, np.x, c);
	}
}

void
win_write_px(const struct win *win, const struct point *p, const struct pixel *px)
{
	set_color(px->fg);
	win_write(win, p, px->c);
	unset_color(px->fg);
}


void
win_write_str(const struct win *win, const struct point *p, const char *str)
{
	const char *cp;
	struct point np = { .x = p->x, .y = p->y };

	for (cp = str; *cp != '\0'; p++) {
		win_write(win, &np, *cp);
		np.x++;
	}
}

void
win_printf(const struct win *win, const struct point *p, const char *fmt, ...)
{
	char buf[255];
	va_list ap;
	size_t i, l;
	struct point np;

	va_start(ap, fmt);
	l = vsnprintf(buf, 255, fmt, ap);
	va_end(ap);

	np = *p;

	for (i = 0; i < l; i++) {
		win_write(win, &np, buf[i]);
		np.x++;
	}
}

void
win_erase(void)
{
	werase(stdscr);
}

void
win_refresh(void)
{
	wrefresh(stdscr);
}
