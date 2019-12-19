#ifndef __PAINTERS_H
#define __PAINTERS_H
#include "sim/world.h"
#include "sim/action.h"
#include "window.h"

void draw_infol(struct win *win, struct point *view, struct point *cursor);
void draw_infor(struct win *win, struct world *w);
void draw_selection(struct win *win, struct point *cursor);
void draw_world(struct win *win, struct world *w, struct point *view);
void draw_actions(struct win *win, struct action *a, size_t len, struct point *view);
#endif
