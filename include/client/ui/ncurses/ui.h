#ifndef CLIENT_UI_NCURSES_UI_H
#define CLIENT_UI_NCURSES_UI_H

#include "client/hiface.h"
#include "client/input/keymap.h"

struct ncurses_ui_ctx;

struct ncurses_ui_ctx *ncurses_ui_init(void);
void ncurses_ui_render(struct ncurses_ui_ctx *nc, struct hiface *hf);
void ncurses_ui_handle_input(struct keymap **km, struct hiface *hf);
struct rectangle ncurses_ui_viewport(struct ncurses_ui_ctx *nc);
void ncurses_ui_deinit(void);
#endif
