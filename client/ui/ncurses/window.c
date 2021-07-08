#include "posix.h"

#ifdef __APPLE__
// POSIX C source doesn't give us SIGWINCH on bsd
#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE
#endif
#endif

#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "client/ui/ncurses/window.h"
#include "shared/input/keyboard.h"
#include "shared/math/geom.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

static struct {
	struct darr wins;
	bool resized;
} term = { 0 };

static void
calc_proportion(struct rectangle *sub, const struct rectangle *par,
	enum win_split sp, double pct, bool primary)
{
	const int *dim = NULL, *coord = NULL;
	int *newdim = NULL, *newcoord = NULL;

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
	struct win *win = _win, *parent = darr_get(&term.wins, win->parent);

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
	darr_for_each(&term.wins, NULL, resize_iterator);
}

bool
term_check_resize(void)
{
	if (!term.resized) {
		return false;
	}

	struct win *root_win = darr_get(&term.wins, 0);

	get_term_dimensions(&root_win->rect.height, &root_win->rect.width);

	resize_term(root_win->rect.height, root_win->rect.width);
	wresize(stdscr, root_win->rect.height, root_win->rect.width);

	L(log_misc, "terminal changed size: %dx%d", root_win->rect.height, root_win->rect.width);

	term_commit_layout();
	term.resized = false;
	wclear(stdscr);

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
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	wbkgdset(stdscr, ' ');
	curs_set(0);

	mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
	mouseinterval(0);

	win_init(&root_win);
	get_term_dimensions(&root_win.rect.height, &root_win.rect.width);

	memset(&term, 0, sizeof(term));
	darr_init(&term.wins, sizeof(struct win));
	darr_push(&term.wins, &root_win);

	L(log_misc, "setup root window");

	install_signal_handler();
}

void
term_teardown(void)
{
	darr_destroy(&term.wins);
	endwin();
}

struct win *
win_create(struct win *parent)
{
	struct win win, *winp;
	size_t i;

	if (parent == NULL) {
		parent = darr_get(&term.wins, 0);
	}

	win_init(&win);
	win.parent = parent->index;

	i = darr_push(&term.wins, &win);
	winp = darr_get(&term.wins, i);
	winp->index = i;

	if (parent->children[0] == 0) {
		parent->children[0] = i;
	} else {
		parent->children[1] = i;
	}

	return winp;
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
	wattr_set(stdscr, px->attr, px->clr, NULL);
	win_write(win, p, px->c);
}

void
win_clr_attr(void)
{
	wattr_set(stdscr, A_NORMAL, 0, NULL);
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

size_t
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

	return l;
}

void
win_clrtoeol(const struct win *win, const struct point *p)
{
	struct point np = *p;

	for (; np.x < win->rect.width; ++np.x) {
		win_write(win, &np, ' ');
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

short
setup_color_pair(struct graphics_t *g, short f, short b)
{
	short num = g->color_i++;

	if (init_pair(num, f, b) != 0) {
		L(log_misc, "failed to initialize color pair");
	}

	return num;
}

uint64_t
attr_transform(uint8_t attr)
{
	size_t i;
	uint64_t ret = 0, map[] = {
		A_NORMAL,
		A_STANDOUT,
		A_UNDERLINE,
		A_REVERSE,
		A_BLINK,
		A_DIM,
		A_BOLD,
		A_INVIS,
	};

	for (i = 0; i < 8; ++i) {
		if (attr & (1 << i)) {
			ret |= map[i];
		}
	}

	return ret;
}

static unsigned
transform_key(unsigned k, bool esc)
{
	switch (k) {
	case KEY_UP:
		return skc_up;
	case KEY_DOWN:
		return skc_down;
	/* case KEY_SR: */
	/* 	return skc_shift_up; */
	/* case KEY_SF: */
	/* 	return skc_shift_down; */
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
	case KEY_HOME:
		return skc_home;
	case KEY_END:
		return skc_end;
	case KEY_PPAGE:
		return skc_pgup;
	case KEY_NPAGE:
		return skc_pgdn;
	default:
		return k;
	}
}

void
term_win_poll_events(void *ctx, win_key_cb key_cb, win_mouse_cb mouse_cb)
{
	int key;
	uint8_t k;
	bool esc = false;
	static float old_x, old_y;
	static bool initialized = false;

	while ((key = getch()) != ERR) {
		if (key == 033) {
			esc = true;
			continue;
		} else if (key == KEY_MOUSE) {
			MEVENT event;
			if (getmouse(&event) == OK) {
				if (initialized) {
					float dx = -(old_x - event.x),
					      dy = -(old_y - event.y);
					if (!(event.bstate & BUTTON_SHIFT)) {
						mouse_cb(ctx, dx, dy);
					}
				} else {
					initialized = true;
				}

				old_x = event.x;
				old_y = event.y;

				uint32_t i;
				uint8_t transform[4] = { 0, skc_mb1, skc_mb3, skc_mb2 };
				for (i = 1; i < 4; ++i) {
					if (BUTTON_PRESS(event.bstate, i)) {
						key_cb(ctx, transform[i], 0, key_action_press);
					} else if (BUTTON_RELEASE(event.bstate, i)) {
						key_cb(ctx, transform[i], 0, key_action_release);
					}
				}
			}
		}

		k = transform_key(key, esc);

		if (k < 255) {
			key_cb(ctx, k, 0, key_action_oneshot);
		}

		esc = false;
	}

	if (esc) {
		key_cb(ctx, 033, 0, key_action_oneshot);
	}
}
