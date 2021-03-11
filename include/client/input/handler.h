#ifndef CLIENT_INPUT_HANDLER_H
#define CLIENT_INPUT_HANDLER_H
#include "client/cfg/keymap.h"
#include "client/client.h"

typedef void (*kc_func)(struct client *);

struct keymap *handle_input(struct keymap *km, unsigned k, struct client *cli);
void trigger_cmd_with_num(enum key_command kc, struct client *cli, int32_t val);
void trigger_cmd(enum key_command kc, struct client *cli);
#endif
