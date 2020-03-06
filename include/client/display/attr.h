#ifndef CLIENT_DISPLAY_ATTR_H
#define CLIENT_DISPLAY_ATTR_H

#include <stdint.h>

struct _ansi_palette {
	short no;
	short black;
	short red;
	short green;
	short yellow;
	short blue;
	short magenta;
	short cyan;
	short white;
};

struct attrs {
	struct _ansi_palette fg;
	struct _ansi_palette bg;
	uint32_t normal;
	uint32_t standout;
	uint32_t underline;
	uint32_t reverse;
	uint32_t blink;
	uint32_t dim;
	uint32_t bold;
	uint32_t protect;
	uint32_t invis;
	uint32_t altcharset;
	uint32_t italic;
};

extern struct attrs attr;

void attr_init(void);
#endif
