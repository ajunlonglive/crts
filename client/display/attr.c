#include <curses.h>

#include "client/display/attr.h"

struct attrs attr = {
	.normal = A_NORMAL,
	.standout = A_STANDOUT,
	.underline = A_UNDERLINE,
	.reverse = A_REVERSE,
	.blink = A_BLINK,
	.dim = A_DIM,
	.bold = A_BOLD,
	.invis = A_INVIS,
};

_Static_assert(A_NORMAL == 0, "a_normal is not 0");

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
