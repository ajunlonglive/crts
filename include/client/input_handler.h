#ifndef CLIENT_INPUT_HANDLER_H
#define CLIENT_INPUT_HANDLER_H
#include "client/client.h"

#include "shared/util/inih.h"
#include "shared/input/keyboard.h"

typedef void ((*kc_func)(struct client *, uint32_t arg));

struct input_command_name {
	const char *name;
	kc_func func;
};

struct keymap {
	kc_func func;
	uint32_t arg;
	uint8_t next_layer;
	uint8_t key, mod, action;
};

#define LAYER_MAX 64

struct keymap_layer {
	struct keymap maps[LAYER_MAX];
	uint8_t len;
};

/* void center_cursor(struct client *cli, uint32_t _); */
/* void constrain_cursor(rect ref, struct point *curs); */

void register_input_commands(const struct input_command_name *alt);
void register_input_constants(const struct cfg_lookup_table *ltbl);
bool input_command_name_lookup(const char *name, kc_func *res);
bool input_constant_lookup(const char *name, uint32_t *res);

struct keymap *km_find_or_create_map(struct keymap_layer *layer, uint8_t key, uint8_t mod, uint8_t action);
struct keymap_layer *km_current_layer(void);
void km_add_layer(uint8_t *res);
void km_set_layer(uint8_t l);

void input_handle_key(struct client *cli, uint8_t key, uint8_t mod, enum key_action action);
void input_handle_mouse(struct client *cli, float dx, float dy);
void input_init(void);
#endif
