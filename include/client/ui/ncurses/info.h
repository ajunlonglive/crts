#ifndef CLIENT_UI_NCURSES_INFO_H
#define CLIENT_UI_NCURSES_INFO_H

#include "client/hiface.h"
#include "client/ui/ncurses/window.h"

void draw_infol(struct win *win, struct hiface *hif);
void draw_infor(struct win *win, struct hiface *hif);
#endif
