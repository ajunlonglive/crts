#ifndef CLIENT_UI_NCURSES_INFO_H
#define CLIENT_UI_NCURSES_INFO_H

#include "client/client.h"
#include "client/ui/ncurses/window.h"

void draw_infol(struct win *win, struct client *cli);
void draw_infor(struct win *win, struct client *cli);
#endif
