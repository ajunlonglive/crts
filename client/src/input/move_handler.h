#ifndef __INPUT_MOVE_HANDLER_H
#define __INPUT_MOVE_HANDLER_H
void cursor_up(void *d);
void cursor_down(void *d);
void cursor_left(void *d);
void cursor_right(void *d);
void view_up(void *d);
void view_down(void *d);
void view_left(void *d);
void view_right(void *d);
void end_simulation(void *disp);
void set_input_mode_select(void *disp);
void set_input_mode_normal(void *disp);
#endif
