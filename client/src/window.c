#define _POSIX_C_SOURCE 199309L

#include <curses.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "window.h"
#include "log.h"

_Atomic int draw_mutex_locked = 0;

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
		split_dim = &win->rect.width;
		break;
	default:
		L("split -");
		split_dim = &win->rect.height;
		break;
	}

	curx = win->rect.pos.x;
	cury = win->rect.pos.y;

	L("win has %d children", win->ccnt);
	if (win->ccnt > 1) {
		main_size = win->main_win_pct * (double)(*split_dim);
		main_size += (*split_dim - main_size) % (win->ccnt - 1);
		sub_size = (double)((*split_dim) - main_size) / (win->ccnt - 1);

		for (i = 0; i < win->ccnt; i++) {
			win->children[i]->rect.pos.x = curx;
			win->children[i]->rect.pos.y = cury;

			switch (win->split) {
			case 1:
				win->children[i]->rect.width = i == 0 ? main_size : sub_size;
				win->children[i]->rect.height = win->rect.height;
				curx += win->children[i]->rect.width;
				break;
			default:
				win->children[i]->rect.width = win->rect.width;
				win->children[i]->rect.height = i == 0 ? main_size : sub_size;
				cury += win->children[i]->rect.height;
				break;
			}

			L("child #%d moved to x: %d, y: %d, width: %d, height: %d",
			  i,
			  win->children[i]->rect.pos.x,
			  win->children[i]->rect.pos.y,
			  win->children[i]->rect.width,
			  win->children[i]->rect.height
			  );

			win_changed_size(win->children[i]);
		}
	} else {
		win->children[0]->rect.pos.x = curx;
		win->children[0]->rect.pos.y = cury;
		win->children[0]->rect.width = win->rect.width;
		win->children[0]->rect.height = win->rect.height;
		L("child moved to x: %d, y: %d, width: %d, height: %d",
		  win->children[0]->rect.pos.x,
		  win->children[0]->rect.pos.y,
		  win->children[0]->rect.width,
		  win->children[0]->rect.height
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

	get_term_dimensions(&root_win->rect.height, &root_win->rect.width);

	resize_term(root_win->rect.height, root_win->rect.width);
	wresize(stdscr, root_win->rect.height, root_win->rect.width);

	L("terminal changed size: %dx%d", root_win->rect.height, root_win->rect.width);
	win_changed_size(root_win);
	L("done resizing");
	wclear(stdscr);
	win_refresh(root_win);
	L("done update");
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

void term_setup(void)
{
	struct win *win = win_alloc();

	cbreak();
	noecho();
	nonl();
	initscr();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	// hide cursor
	curs_set(0);

	root_win = win;
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
	size_t i;

	if (win->ccnt > 1)
		for (i = 0; i < win->ccnt; i++)
			win_destroy(win->children[i]);

	free(win->children);
	free(win);
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
	werase(stdscr);
	repaint_rec(win);
	wrefresh(stdscr);
}
