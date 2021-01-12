#ifndef CLIENT_INPUT_HELPERS_H
#define CLIENT_INPUT_HELPERS_H

#include "client/client.h"

long client_get_num(struct client *cli, long def);
void override_num_arg(struct client *cli, long num);
void cli_describe(struct client *cli, enum keymap_category cat, char *desc, ...);
void client_reset_input(struct client *cli);
void clib_append_char(struct client_buf *hbf, unsigned c);
void check_selection_resize(struct client *cli);
void constrain_cursor(struct rectangle *ref, struct point *curs);
void resize_selection_start(struct client *cli);
void resize_selection_stop(struct client *cli);
void move_viewport(struct client *cli, int32_t dx, int32_t dy);
void client_init_view(struct client *cli, struct point *p);
#endif
