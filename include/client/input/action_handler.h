#ifndef CLIENT_INPUT_ACTION_HANDLER_H
#define CLIENT_INPUT_ACTION_HANDLER_H
#include "client/hiface.h"

void set_action_type(struct hiface *hif);

void resize_selection(struct hiface *hif);

void set_action_target(struct hiface *hif);
void exec_action(struct hiface *hif);
void undo_last_action(struct hiface *hif);
#endif
