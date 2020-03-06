#define _POSIX_C_SOURCE 201900L

#include <time.h>

#include "client/display/container.h"
#include "client/display/info.h"
#include "client/display/world.h"
#include "client/hiface.h"
#include "shared/util/log.h"

#define FPS 30

static void
fix_cursor(const struct rectangle *r, struct point *vu, struct point *cursor)
{
	int diff;

	if (point_in_rect(cursor, r)) {
		return;
	}

	if ((diff = 0 - cursor->y) > 0 || (diff = (r->height - 1) - cursor->y) < 0) {
		vu->y -= diff;
		cursor->y += diff;
	}

	if ((diff = 0 - cursor->x) > 0 || (diff = (r->width - 1) - cursor->x) < 0) {
		vu->x -= diff;
		cursor->x += diff;
	}
}

void
draw(struct display_container *dc, struct hiface *hf)
{
	term_check_resize();

	if (hf->im == im_select) {
		fix_cursor(&dc->root.world->rect, &hf->view, &hf->cursor);
	}

	hf->redrew_world = draw_world(dc->root.world, hf);

	win_clr_attr();

	draw_infol(dc->root.info.l, hf);

	draw_infor(dc->root.info.r, hf);

}
