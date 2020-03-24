#ifndef __INPUT_ACTION_HANDLER_H
#define __INPUT_ACTION_HANDLER_H
#include "client/hiface.h"

void set_action_type(struct hiface *hif);
void set_action_radius(struct hiface *hif);
void action_radius_expand(struct hiface *hif);
void action_radius_shrink(struct hiface *hif);
void set_action_target(struct hiface *hif);
void exec_action(struct hiface *hif);
#endif
