#ifndef CLIENT_INPUT_ACTION_HANDLER_H
#define CLIENT_INPUT_ACTION_HANDLER_H
#include "client/hiface.h"

void set_action_type(struct hiface *hif);
void set_action_radius(struct hiface *hif);
void set_action_source(struct hiface *hif);
void swap_cursor_with_source(struct hiface *hif);
void action_radius_expand(struct hiface *hif);
void action_radius_shrink(struct hiface *hif);
void set_action_target(struct hiface *hif);
void read_action_target(struct hiface *hif);
void exec_action(struct hiface *hif);
void toggle_action_flag(struct hiface *hif);
#endif
