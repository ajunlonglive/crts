#include <stdlib.h>

#include "client/hiface.h"
#include "client/input/handler.h"
#include "client/input/move_handler.h"
#include "shared/util/log.h"

#define DEF_MOVE_AMNT 1

void
center(struct hiface *d)
{
	d->view.x = 0;
	d->view.y = 0;
}

void
cursor_up(struct hiface *d)
{
	d->cursor.y -= hiface_get_num(d, DEF_MOVE_AMNT);
}

void
cursor_down(struct hiface *d)
{
	d->cursor.y += hiface_get_num(d, DEF_MOVE_AMNT);
}

void
cursor_left(struct hiface *d)
{
	d->cursor.x -= hiface_get_num(d, DEF_MOVE_AMNT);
}

void
cursor_right(struct hiface *d)
{
	d->cursor.x += hiface_get_num(d, DEF_MOVE_AMNT);
}

void
view_up(struct hiface *d)
{
	d->view.y -= hiface_get_num(d, DEF_MOVE_AMNT);
}

void
view_down(struct hiface *d)
{
	d->view.y += hiface_get_num(d, DEF_MOVE_AMNT);
}

void
view_left(struct hiface *d)
{
	d->view.x -= hiface_get_num(d, DEF_MOVE_AMNT);
}

void
view_right(struct hiface *d)
{
	d->view.x += hiface_get_num(d, DEF_MOVE_AMNT);
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
