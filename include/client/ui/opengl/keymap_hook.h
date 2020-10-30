#ifndef INCLUDE_CLIENT_UI_OPENGL_KEYMAP_HOOK_H
#define INCLUDE_CLIENT_UI_OPENGL_KEYMAP_HOOK_H

#include "client/cfg/keymap.h"

enum keymap_hook_result opengl_ui_keymap_hook(struct opengl_ui_ctx *ctx,
	char *err, const char *sec, const char *k, const char *v, uint32_t line);
#endif
