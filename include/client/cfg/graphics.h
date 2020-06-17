#ifndef CLIENT_CFG_GRAPHICS_H
#define CLIENT_CFG_GRAPHICS_H

#include <stdbool.h>
#include <stdint.h>

enum gfx_cfg_section {
	gfx_cfg_section_global,
	gfx_cfg_section_tiles,
	gfx_cfg_section_entities,
	gfx_cfg_section_cursor,
};

struct parse_graphics_ctx {
	void *ctx;
	bool ((*setup_color)(void *_, int32_t sect, int32_t type,
		char c, short fg, short bg, short attr, short zi));
};

bool parse_graphics_handler(void *ctx, const char *sect, const char *k,
	const char *v, uint32_t line);
#endif
