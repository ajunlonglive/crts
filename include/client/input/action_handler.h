#ifndef CLIENT_INPUT_ACTION_HANDLER_H
#define CLIENT_INPUT_ACTION_HANDLER_H
#include "client/hiface.h"

void set_action_type(struct hiface *hif);

void set_action_width(struct hiface *hif);
void action_width_grow(struct hiface *hif);
void action_width_shrink(struct hiface *hif);
void set_action_height(struct hiface *hif);
void action_height_grow(struct hiface *hif);
void action_height_shrink(struct hiface *hif);

void action_rect_rotate(struct hiface *hif);

void swap_cursor_with_source(struct hiface *hif);
void set_action_target(struct hiface *hif);
void read_action_target(struct hiface *hif);
void exec_action(struct hiface *hif);
void toggle_action_flag(struct hiface *hif);
void undo_last_action(struct hiface *hif);
#endif
