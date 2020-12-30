#ifndef CLIENT_INPUT_ACTION_HANDLER_H
#define CLIENT_INPUT_ACTION_HANDLER_H
#include "client/client.h"

void set_action_type(struct client *cli);

void resize_selection(struct client *cli);

void set_action_target(struct client *cli);
void exec_action(struct client *cli);
void undo_last_action(struct client *cli);
#endif
