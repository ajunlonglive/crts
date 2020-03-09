#include <curses.h>

#include "client/display/attr.h"

enum color_pairs {
	color_no,
	color_black,
	color_red,
	color_green,
	color_yellow,
	color_blue,
	color_magenta,
	color_cyan,
	color_white,
	color_bg_black,
	color_bg_red,
	color_bg_green,
	color_bg_yellow,
	color_bg_blue,
	color_bg_magenta,
	color_bg_cyan,
	color_bg_white
};

struct attrs attr = {
	.fg = {
		.no      = color_no,
		.black   = color_black,
		.red     = color_red,
		.green   = color_green,
		.yellow  = color_yellow,
		.blue    = color_blue,
		.magenta = color_magenta,
		.cyan    = color_cyan,
		.white   = color_white,
	},
	.bg = {
		.no      = color_no,
		.black   = color_bg_black,
		.red     = color_bg_red,
		.green   = color_bg_green,
		.yellow  = color_bg_yellow,
		.blue    = color_bg_blue,
		.magenta = color_bg_magenta,
		.cyan    = color_bg_cyan,
		.white   = color_bg_white,
	},
	.normal = A_NORMAL,
	.standout = A_STANDOUT,
	.underline = A_UNDERLINE,
	.reverse = A_REVERSE,
	.blink = A_BLINK,
	.dim = A_DIM,
	.bold = A_BOLD,
	.protect = A_PROTECT,
	.invis = A_INVIS,
	.altcharset = A_ALTCHARSET,
};

void
attr_init(void)
{
	init_pair(color_no, -1, -1);
	init_pair(color_black, COLOR_BLACK, -1);
	init_pair(color_red, COLOR_RED, -1);
	init_pair(color_green, COLOR_GREEN, -1);
	init_pair(color_yellow, COLOR_YELLOW, -1);
	init_pair(color_blue, COLOR_BLUE, -1);
	init_pair(color_magenta, COLOR_MAGENTA, -1);
	init_pair(color_cyan, COLOR_CYAN, -1);
	init_pair(color_white, COLOR_WHITE, -1);

	init_pair(color_bg_black, -1, COLOR_BLACK);
	init_pair(color_bg_red, -1, COLOR_RED);
	init_pair(color_bg_green, -1, COLOR_GREEN);
	init_pair(color_bg_yellow, -1, COLOR_YELLOW);
	init_pair(color_bg_blue, -1, COLOR_BLUE);
	init_pair(color_bg_magenta, -1, COLOR_MAGENTA);
	init_pair(color_bg_cyan, -1, COLOR_CYAN);
	init_pair(color_bg_white, -1, COLOR_WHITE);
}
