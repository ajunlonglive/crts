#ifndef __WINDOW_H
#define __WINDOW_H
#include "geom.h"
#include <curses.h>

enum win_layout {
	win_layout_full,
	win_layout_80,
	win_layout_70,
};

struct win {
	struct win *parent;
	struct rectangle rect;

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
void win_write_str(const struct win *win, const struct point *p, const char *str);
void win_write(const struct win *win, const struct point *p, char c);
void win_refresh(struct win *win);
void win_printf(const struct win *win, const struct point *p, const char *fmt, ...);
#endif
