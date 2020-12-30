#ifndef CLIENT_INPUT_HANDLER_H
#define CLIENT_INPUT_HANDLER_H
#include "client/cfg/keymap.h"
#include "client/client.h"

typedef void (*kc_func)(struct client *);
typedef void ((*for_each_completion_cb)(void *ctx, struct keymap *km));

struct keymap *handle_input(struct keymap *km, unsigned k, struct client *cli);
void trigger_cmd_with_num(enum key_command kc, struct client *cli, int32_t val);
void trigger_cmd(enum key_command kc, struct client *cli);
void for_each_completion(struct keymap *km, void *ctx, for_each_completion_cb cb);
void describe_completions(struct client *cli, struct keymap *km,
	void *usr_ctx, for_each_completion_cb cb);
#endif
