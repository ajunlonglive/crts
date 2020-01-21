#include "move_handler.h"
#include "handler.h"

#define MOVE_AMNT 8;

void
cursor_up(struct hiface *d)
{
	d->cursor.y -= MOVE_AMNT;
}

void
cursor_down(struct hiface *d)
{
	d->cursor.y += MOVE_AMNT;
}

void
cursor_left(struct hiface *d)
{
	d->cursor.x -= MOVE_AMNT;
}

void
cursor_right(struct hiface *d)
{
	d->cursor.x += MOVE_AMNT;
}

void
view_up(struct hiface *d)
{
	d->view.y -= MOVE_AMNT;
}

void
view_down(struct hiface *d)
{
	d->view.y += MOVE_AMNT;
}

void
view_left(struct hiface *d)
{
	d->view.x -= MOVE_AMNT;
}

void
view_right(struct hiface *d)
{
	d->view.x += MOVE_AMNT;
}

void
end_simulation(struct hiface *d)
{
	d->sim->run = 0;
}

void
set_input_mode_select(struct hiface *d)
{
	d->im = im_select;
}

void
set_input_mode_normal(struct hiface *d)
{
	d->im = im_normal;
}
