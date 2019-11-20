#define _POSIX_C_SOURCE 199309L

#include <curses.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "window.h"
#include "log.h"

static struct win *root_win;
static void repaint_rec(struct win *win);

static void win_changed_size(struct win *win)
{
	size_t i;
	int *split_dim, main_size, sub_size, curx, cury;

	if (win == NULL)
		win = root_win;

	L("resizing window %p", win);

	if (win->ccnt < 1)
		return;


	switch (win->split) {
	case 1:
		L("split |");
		split_dim = &win->width;
		break;
	default:
		L("split -");
		split_dim = &win->height;
		break;
	}

	curx = win->x;
	cury = win->y;

	L("win has %d children", win->ccnt);
	if (win->ccnt > 1) {
		main_size = win->main_win_pct * (double)(*split_dim);
		main_size += (*split_dim - main_size) % (win->ccnt - 1);
		sub_size = (double)((*split_dim) - main_size) / (win->ccnt - 1);

		for (i = 0; i < win->ccnt; i++) {
			win->children[i]->x = curx;
			win->children[i]->y = cury;

			switch (win->split) {
			case 1:
				win->children[i]->width = i == 0 ? main_size : sub_size;
				win->children[i]->height = win->height;
				curx += win->children[i]->width;
				break;
			default:
				win->children[i]->width = win->width;
				win->children[i]->height = i == 0 ? main_size : sub_size;
				cury += win->children[i]->height;
				break;
			}

			L("child #%d moved to x: %d, y: %d, width: %d, height: %d",
			  i,
			  win->children[i]->x,
			  win->children[i]->y,
			  win->children[i]->width,
			  win->children[i]->height
			  );

			win_changed_size(win->children[i]);
		}
	} else {
		win->children[0]->x = curx;
		win->children[0]->y = cury;
		win->children[0]->width = win->width;
		win->children[0]->height = win->height;
		L("child moved to x: %d, y: %d, width: %d, height: %d",
		  win->children[0]->x,
		  win->children[0]->y,
		  win->children[0]->width,
		  win->children[0]->height
		  );

		win_changed_size(win->children[0]);
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
	L("caught SIGWINCH");

	get_term_dimensions(&root_win->height, &root_win->width);

	resize_term(root_win->height, root_win->width);
	wresize(stdscr, root_win->height, root_win->width);

	L("terminal changed size: %dx%d", root_win->height, root_win->width);
	win_changed_size(root_win);
	L("done resizing");
	win_refresh(root_win);
	L("done update");
}

static void install_signal_handler(void)
{
	struct sigaction *sigact;

	sigact = malloc(sizeof(struct sigaction));

	sigact->sa_flags = 0;
	sigact->sa_handler = handle_sigwinch;
	sigaction(SIGWINCH, sigact, NULL);
}

static struct win *win_alloc()
{
	struct win *win;

	win = malloc(sizeof(struct win));

	win->x = 0;
	win->y = 0;
	win->width = 0;
	win->height = 0;
	win->ccnt = 0;
	win->main_win_pct = 1.0;
	win->split = 0;

	win->parent = NULL;
	win->children = NULL;

	return win;
}

void term_setup(void)
{
	struct win *win = win_alloc();

	cbreak();
	noecho();
	nonl();
	initscr();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	root_win = win;
	get_term_dimensions(&root_win->height, &root_win->width);

	L("setup root window");

	install_signal_handler();
}

void term_teardown(void)
{
	endwin();
}

struct win *win_init(struct win *parent)
{
	struct win *win;

	L("initializing a window");

	if (parent == NULL)
		parent = root_win;

	if (parent == NULL) {
		L("you have not called term_setup");
		return NULL;
	}

	win = win_alloc();
	win->parent = parent;

	parent->ccnt++;

	parent->children =
		realloc(parent->children, sizeof(struct win *) * parent->ccnt);

	parent->children[parent->ccnt - 1] = win;

	win_changed_size(parent);
	win_refresh(parent);

	return win;
}

void win_destroy(struct win *win)
{
	free(win);
}

void win_write(struct win *win, int x, int y, char c)
{
	x += win->x;
	y += win->y;

	if (x < 0 || y < 0) {
		L("negative coords: refusing to write %c at (%d, %d)", c, x, y);
		return;
	} else if (x > win->x + win->width) {
		L("x bounds: refusing to write %c at (%d, %d)", c, x, y);
	} else if (y > win->y + win->height) {
		L("y bounds: refusing to write %c at (%d, %d)", c, x, y);
		return;
	}

	mvwaddch(stdscr, y, x, c);
}

void win_write_str(struct win *win, int x, int y, char *str)
{
	char *p;

	for (p = str; *p != '\0'; p++) {
		win_write(win, x, y, *p);
		x++;
	}
}

static void repaint_rec(struct win *win)
{
	size_t i;

	if (win->ccnt == 0 && win->painter != NULL) {
		win->painter(win);
		return;
	}

	for (i = 0; i < win->ccnt; i++)
		repaint_rec(win->children[i]);
}

void win_refresh(struct win *win)
{
	wclear(stdscr);
	repaint_rec(win);
	wrefresh(stdscr);
}
