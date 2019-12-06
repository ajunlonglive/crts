#define _POSIX_C_SOURCE 199309L

#include <curses.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "window.h"
#include "util/log.h"
#include "math/geom.h"

static struct win *root_win;

static void win_changed_size(struct win *win)
{
	int *split_dim, main_size, sub_size;
	size_t i;
	struct point curpos;

	//L("resizing window %p with %d children", win, win->ccnt);

	curpos = win->rect.pos;

	if (win->ccnt < 1) {
		return;
	} else if (win->ccnt == 1) {
		win->children[0]->rect.pos = curpos;
		win->children[0]->rect.width = win->rect.width;
		win->children[0]->rect.height = win->rect.height;

		win_changed_size(win->children[0]);
		return;
	}

	if (win->split == 1)
		split_dim = &win->rect.width; // vertical split
	else
		split_dim = &win->rect.height; // horizontal split

	main_size = win->main_win_pct * (double)(*split_dim);
	main_size += (*split_dim - main_size) % (win->ccnt - 1);
	sub_size = (double)((*split_dim) - main_size) / (win->ccnt - 1);

	for (i = 0; i < win->ccnt; i++) {
		win->children[i]->rect.pos = curpos;

		if (win->split == 1) {
			win->children[i]->rect.width = i == 0 ? main_size : sub_size;
			win->children[i]->rect.height = win->rect.height;
			curpos.x += win->children[i]->rect.width;
		} else {
			win->children[i]->rect.width = win->rect.width;
			win->children[i]->rect.height = i == 0 ? main_size : sub_size;
			curpos.y += win->children[i]->rect.height;
		}

		win_changed_size(win->children[i]);
	}
};

static void get_term_dimensions(int *height, int *width)
{
	struct winsize w;

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	*width = w.ws_col;
	*height = w.ws_row;
}

static void handle_sigwinch(int _)
{
	get_term_dimensions(&root_win->rect.height, &root_win->rect.width);

	resize_term(root_win->rect.height, root_win->rect.width);
	wresize(stdscr, root_win->rect.height, root_win->rect.width);

	L("terminal changed size: %dx%d", root_win->rect.height, root_win->rect.width);
	win_changed_size(root_win);
	wclear(stdscr);
}

static void install_signal_handler(void)
{
	struct sigaction sigact;

	memset(&sigact, 0, sizeof(struct sigaction));

	sigact.sa_flags = 0;
	sigact.sa_handler = handle_sigwinch;
	sigaction(SIGWINCH, &sigact, NULL);
}

static struct win *win_alloc()
{
	struct win *win;

	win = malloc(sizeof(struct win));
	memset(win, 0, sizeof(struct win));

	win->main_win_pct = 1.0;
	win->split = 0;

	win->parent = NULL;
	win->children = NULL;

	return win;
}

static void init_color_pairs()
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

void term_setup(void)
{
	cbreak();
	noecho();
	nonl();
	initscr();
	start_color();
	use_default_colors();
	init_color_pairs();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	wbkgdset(stdscr, ' ');
	curs_set(0); // hide cursor

	root_win = win_alloc();
	get_term_dimensions(&root_win->rect.height, &root_win->rect.width);

	L("setup root window");

	install_signal_handler();
}

void term_teardown(void)
{
	win_destroy(root_win);
	endwin();
}

struct win *win_init(struct win *parent)
{
	struct win *win;

	if (parent == NULL)
		parent = root_win;

	win = win_alloc();
	win->parent = parent;

	parent->ccnt++;

	parent->children =
		realloc(parent->children, sizeof(struct win *) * parent->ccnt);

	parent->children[parent->ccnt - 1] = win;

	win_changed_size(parent);

	return win;
}

void win_destroy(struct win *win)
{
	size_t i;

	if (win->ccnt > 1)
		for (i = 0; i < win->ccnt; i++)
			win_destroy(win->children[i]);

	free(win->children);
	free(win);
}

void set_color(enum color c)
{
	attron(COLOR_PAIR(c));
}

void unset_color(enum color c)
{
	attroff(COLOR_PAIR(c));
}

void win_write(const struct win *win, const struct point *p, char c)
{
	struct point np = {
		.x = win->rect.pos.x + p->x,
		.y = win->rect.pos.y + p->y,
	};

	if (point_in_rect(&np, &win->rect))
		mvwaddch(stdscr, np.y, np.x, c);
}

void win_write_str(const struct win *win, const struct point *p, const char *str)
{
	const char *cp;
	struct point np = { .x = p->x, .y = p->y };

	for (cp = str; *cp != '\0'; p++) {
		win_write(win, &np, *cp);
		np.x++;
	}
}

void win_printf(const struct win *win, const struct point *p, const char *fmt, ...)
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

void win_erase()
{
	werase(stdscr);
}
void win_refresh()
{
	wrefresh(stdscr);
}
