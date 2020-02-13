#ifndef __WINDOW_H
#define __WINDOW_H
#include <stdlib.h>

#include "shared/types/geom.h"

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

	size_t ccnt;
	struct win **children;
};

enum color {
	color_no,
	color_blk,
	color_red,
	color_grn,
	color_ylw,
	color_blu,
	color_mag,
	color_cyn,
	color_wte,
	color_bg_blk,
	color_bg_red,
	color_bg_grn,
	color_bg_ylw,
	color_bg_blu,
	color_bg_mag,
	color_bg_cyn,
	color_bg_wte
};

void term_setup(void);
void term_teardown(void);

void set_color(enum color c);
void unset_color(enum color c);

struct win *win_init(struct win *parent);
void win_destroy(struct win *win);

void win_write(const struct win *win, const struct point *p, char c);
void win_write_str(const struct win *win, const struct point *p, const char *str);
void win_printf(const struct win *win, const struct point *p, const char *fmt, ...);

void win_erase(void);
void win_refresh(void);
#endif
