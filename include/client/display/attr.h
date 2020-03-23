#ifndef CLIENT_DISPLAY_ATTR_H
#define CLIENT_DISPLAY_ATTR_H

#include <stdint.h>

#define CLR_BG_MASK 0x1

enum color_pairs {
	color_no         = 0 << 1,
	color_black      = 1 << 1,
	color_red        = 2 << 1,
	color_green      = 3 << 1,
	color_yellow     = 4 << 1,
	color_blue       = 5 << 1,
	color_magenta    = 6 << 1,
	color_cyan       = 7 << 1,
	color_white      = 8 << 1,
	color_bg_black   = color_black | CLR_BG_MASK,
	color_bg_red     = color_red | CLR_BG_MASK,
	color_bg_green   = color_green | CLR_BG_MASK,
	color_bg_yellow  = color_yellow | CLR_BG_MASK,
	color_bg_blue    = color_blue | CLR_BG_MASK,
	color_bg_magenta = color_magenta | CLR_BG_MASK,
	color_bg_cyan    = color_cyan | CLR_BG_MASK,
	color_bg_white   = color_white | CLR_BG_MASK,
};

struct attrs {
	uint32_t normal;
	uint32_t standout;
	uint32_t underline;
	uint32_t reverse;
	uint32_t blink;
	uint32_t dim;
	uint32_t bold;
	uint32_t invis;
};

extern struct attrs attr;

void attr_init(void);
#endif
