#include "posix.h"

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
// POSIX C source doesn't give us SIGWINCH on bsd
#define _DARWIN_C_SOURCE
#endif

#include <assert.h>
#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "shared/ui/term/window.h"
#include "shared/input/keyboard.h"
#include "shared/math/geom.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

struct term_win {
	uint32_t parent;
	uint32_t index;
	uint32_t children[2];

	struct rectangle rect;

	double split_pct;
	enum term_win_split split;
};


#define MAX_WINS 10

static struct {
	struct term_win wins[MAX_WINS];
	uint32_t winlen;
	bool resized;
} term = { 0 };

static void
calc_proportion(struct rectangle *sub, const struct rectangle *par, enum term_win_split sp, double pct, bool primary)
{
	const int *dim = NULL, *coord = NULL;
	int *newdim = NULL, *newcoord = NULL;

	switch (sp) {
	case term_win_split_vertical:
		dim = &par->width;
		newdim = &sub->width;
		coord = &par->pos.x;
		newcoord = &sub->pos.x;
		break;
	case term_win_split_horizontal:
		dim = &par->height;
		newdim = &sub->height;
		coord = &par->pos.y;
		newcoord = &sub->pos.y;
		break;
	}

	uint32_t primary_size = pct * (double)(*dim);

	if (primary) {
		*newdim = primary_size;
	} else {
		*newdim = *dim - primary_size;
		*newcoord = *coord + primary_size;
	}
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
	bool primary;
	struct term_win *win, *parent;
	uint32_t i;

	for (i = 1; i < term.winlen; ++i) {
		win = &term.wins[i];
		parent = &term.wins[win->parent];

		win->rect = parent->rect;

		if (parent->children[1] == 0) {
			continue;
		}

		primary = win->index == 0 || parent->children[0] == win->index;

		calc_proportion(&win->rect, &parent->rect, parent->split,
			parent->split_pct, primary);
	}
}

bool
term_check_resize(void)
{
	if (!term.resized) {
		return false;
	}

	struct term_win *root_win = &term.wins[0];

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

void
term_setup(void)
{
	memset(&term, 0, sizeof(term));

	struct term_win *root_win = &term.wins[0];
	*root_win = (struct term_win){ .split_pct = 1.0 };
	get_term_dimensions(&root_win->rect.height, &root_win->rect.width);
	++term.winlen;

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

	install_signal_handler();
}

void
term_teardown(void)
{
	endwin();
}

uint32_t
term_win_create_root(double split_pct, enum term_win_split split)
{
	term.wins[0].split_pct = split_pct;
	term.wins[0].split = split;
	return 0;
}

uint32_t
term_win_create(uint32_t parent, double split_pct, enum term_win_split split)
{
	struct term_win *win;
	uint32_t i;

	assert(term.winlen < MAX_WINS && "too many term windows created");

	i = term.winlen;
	++term.winlen;

	win = &term.wins[i];
	*win = (struct term_win) {
		.parent = parent,
		.index = i,
		.split_pct = split_pct,
		.split = split,
	};

	if (term.wins[parent].children[0] == 0) {
		term.wins[parent].children[0] = i;
	} else {
		term.wins[parent].children[1] = i;
	}

	return i;
}

const struct rectangle *
term_win_rect(uint32_t win_id)
{
	return &term.wins[win_id].rect;
}

void
term_write(uint32_t win_id, const struct point *p, char c)
{
	struct point np = {
		.x = term.wins[win_id].rect.pos.x + p->x,
		.y = term.wins[win_id].rect.pos.y + p->y,
	};

	if (point_in_rect(&np, &term.wins[win_id].rect)) {
		mvwaddch(stdscr, np.y, np.x, c);
	}
}

void
term_write_px(uint32_t win_id, const struct point *p, const struct term_pixel *px)
{
	wattr_set(stdscr, px->attr, px->clr, NULL);
	term_write(win_id, p, px->c);
}

void
term_clear_attr(void)
{
	wattr_set(stdscr, A_NORMAL, 0, NULL);
}

void
term_write_str(uint32_t win_id, const struct point *p, const char *str)
{
	const char *cp;
	struct point np = { .x = p->x, .y = p->y };

	for (cp = str; *cp != '\0'; p++) {
		term_write(win_id, &np, *cp);
		np.x++;
	}
}

uint32_t
term_printf(uint32_t win_id, const struct point *p, const char *fmt, ...)
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
		term_write(win_id, &np, buf[i]);
		np.x++;
	}

	return l;
}

void
term_clrtoeol(uint32_t win_id, const struct point *p)
{
	struct point np = *p;

	for (; np.x < term.wins[win_id].rect.width; ++np.x) {
		term_write(win_id, &np, ' ');
	}
}

void
term_clear(void)
{
	werase(stdscr);
}

void
term_swap_buffers(void)
{
	wrefresh(stdscr);
}

void
term_setup_color_pair(short f, short b, uint32_t id)
{
	if (init_pair(id, f, b) != 0) {
		LOG_W(log_cli, "failed to initialize color pair");
	}
}

uint64_t
term_attr_transform(uint8_t attr)
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
term_poll_events(void *ctx, term_key_cb key_cb, term_mouse_cb mouse_cb)
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
