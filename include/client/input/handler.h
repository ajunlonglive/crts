#ifndef CLIENT_INPUT_HANDLER_H
#define CLIENT_INPUT_HANDLER_H
#include "client/cfg/keymap.h"
#include "client/hiface.h"

typedef void (*kc_func)(struct hiface *);

struct keymap *handle_input(struct keymap *km, unsigned k, struct hiface *hif);
void trigger_cmd(kc_func func, struct hiface *hf);
#endif
