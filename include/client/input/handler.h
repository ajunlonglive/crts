#ifndef CLIENT_INPUT_HANDLER_H
#define CLIENT_INPUT_HANDLER_H
#include "client/cfg/keymap.h"
#include "client/hiface.h"

typedef void (*kc_func)(struct hiface *);
typedef void ((*for_each_completion_cb)(void *ctx, struct keymap *km));

struct keymap *handle_input(struct keymap *km, unsigned k, struct hiface *hif);
void trigger_cmd_with_num(enum key_command kc, struct hiface *hf, int32_t val);
void trigger_cmd(enum key_command kc, struct hiface *hf);
void for_each_completion(struct keymap *km, void *ctx, for_each_completion_cb cb);
void describe_completions(struct hiface *hf, struct keymap *km,
	void *usr_ctx, for_each_completion_cb cb);
#endif
