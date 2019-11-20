#ifndef __WINDOW_H
#define __WINDOW_H
#include <curses.h>

enum win_layout {
	win_layout_full,
	win_layout_80,
	win_layout_70,
};

struct win {
	struct win *parent;
	int x;
	int y;
	int width;
	int height;

	double main_win_pct;
	int split;

	void (*painter)(struct win *);

	size_t ccnt;
	struct win **children;
};

void term_setup(void);
void term_teardown(void);
struct win *win_init(struct win *parent);
void win_prep_canvas(struct win *win);
void win_destroy(struct win *win);
void win_teardown(struct win *win);
void win_write(struct win *win, int x, int y, char c);
void win_write_str(struct win *win, int x, int y, char *str);
void win_refresh(struct win *win);
#endif
