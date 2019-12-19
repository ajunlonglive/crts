#include "move_handler.h"
#include "handler.h"

#define MOVE_AMNT 8;

void cursor_up(void *d)
{
	((struct display *)d)->cursor.y -= MOVE_AMNT;
}

void cursor_down(void *d)
{
	((struct display *)d)->cursor.y += MOVE_AMNT;
}

void cursor_left(void *d)
{
	((struct display *)d)->cursor.x -= MOVE_AMNT;
}

void cursor_right(void *d)
{
	((struct display *)d)->cursor.x += MOVE_AMNT;
}

void view_up(void *d)
{
	((struct display *)d)->view.y -= MOVE_AMNT;
}

void view_down(void *d)
{
	((struct display *)d)->view.y += MOVE_AMNT;
}

void view_left(void *d)
{
	((struct display *)d)->view.x -= MOVE_AMNT;
}

void view_right(void *d)
{
	((struct display *)d)->view.x += MOVE_AMNT;
}

void end_simulation(void *disp)
{
	((struct display *)disp)->sim->run = 0;
}

void set_input_mode_select(void *disp)
{
	((struct display *)disp)->im = im_select;
}

void set_input_mode_normal(void *disp)
{
	((struct display *)disp)->im = im_normal;
}
