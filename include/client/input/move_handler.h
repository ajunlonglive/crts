#ifndef CLIENT_INPUT_MOVE_HANDLER_H
#define CLIENT_INPUT_MOVE_HANDLER_H
#include "client/hiface.h"

void center(struct hiface *d);
void cursor_up(struct hiface *d);
void cursor_down(struct hiface *d);
void cursor_left(struct hiface *d);
void cursor_right(struct hiface *d);
void view_up(struct hiface *d);
void view_down(struct hiface *d);
void view_left(struct hiface *d);
void view_right(struct hiface *d);
void end_simulation(struct hiface *disp);
void set_input_mode_select(struct hiface *disp);
void set_input_mode_normal(struct hiface *disp);
#endif
