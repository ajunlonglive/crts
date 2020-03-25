#ifndef CLIENT_DISPLAY_WINDOW_H
#define CLIENT_DISPLAY_WINDOW_H
#include <stdlib.h>

#include "client/graphics.h"
#include "shared/types/geom.h"

enum win_split {
	ws_vertical,
	ws_horizontal
};

struct win {
	size_t parent;
	size_t index;
	size_t children[2];

	struct rectangle rect;

	double split_pct;
	enum win_split split;
};

void term_setup(void);
void term_teardown(void);
void term_commit_layout(void);
bool term_check_resize(void);

struct win *win_create(struct win *parent);

void win_write(const struct win *win, const struct point *p, char c);
void win_write_px(const struct win *win, const struct point *p, const struct pixel *px);
void win_write_str(const struct win *win, const struct point *p, const char *str);
size_t win_printf(const struct win *win, const struct point *p, const char *fmt, ...);
void win_clrtoeol(const struct win *win, const struct point *p);

void win_erase(void);
void win_refresh(void);
void win_clr_attr(void);
short setup_color_pair(struct graphics_t *g, short f, short b);
uint64_t attr_transform(uint8_t attr);
#endif
