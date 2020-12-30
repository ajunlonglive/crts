#ifndef CLIENT_INPUT_MOVE_HANDLER_H
#define CLIENT_INPUT_MOVE_HANDLER_H
#include "client/client.h"

void find(struct client *d);
void center(struct client *d);
void center_cursor(struct client *d);
void cursor_up(struct client *d);
void cursor_down(struct client *d);
void cursor_left(struct client *d);
void cursor_right(struct client *d);
void view_up(struct client *d);
void view_down(struct client *d);
void view_left(struct client *d);
void view_right(struct client *d);
#endif
