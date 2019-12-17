#define _POSIX_C_SOURCE 201900L

#include <time.h>
#include <curses.h>

#include "messaging/client_message.h"
#include "sim/chunk.h"
#include "types/queue.h"
#include "display/container.h"
#include "display/painters.h"
#include "display/window.h"
#include "display.h"
#include "input/handler.h"
#include "cfg/keymap.h"
#include "math/geom.h"
#include "types/hash.h"
#include "util/log.h"
#include "util/mem.h"

#define FPS 30
#define REQUEST_COOLDOWN 30

struct reqd_chunks {
	struct hash *h;
	int *e;
	size_t len;
	size_t cap;
};

static void fix_cursor(const struct rectangle *r, struct point *vu, struct point *cursor)
{
	int diff;

	if (point_in_rect(cursor, r))
		return;

	if ((diff = 0 - cursor->y) > 0 || (diff = (r->height - 1) - cursor->y) < 0) {
		vu->y -= diff;
		cursor->y += diff;
	}

	if ((diff = 0 - cursor->x) > 0 || (diff = (r->width - 1) - cursor->x) < 0) {
		vu->x -= diff;
		cursor->x += diff;
	}
}

static void request_missing_chunks(struct reqd_chunks *rq, struct display *disp, const struct rectangle *r, struct point *view)
{
	struct point onp, np = onp = nearest_chunk(view);
	struct client_message *cm;
	const int *ip;
	int j;

	union {
		void **vp;
		int **ip;
	} mem = { .ip = &rq->e };

	for (; np.x < view->x + r->width; np.x += CHUNK_SIZE)
		for (np.y = onp.y; np.y < view->y + r->height; np.y += CHUNK_SIZE)
			if (hash_get(disp->sim->w->chunks, &np) == NULL) {
				ip = hash_get(rq->h, &np);

				if (ip == NULL || *ip > REQUEST_COOLDOWN) {
					L("requesting chunk @ %d, %d", np.x, np.y);
					cm = cm_create(client_message_chunk_req, &np);
					queue_push(disp->sim->outbound, cm);

					if (ip == NULL) {
						j = get_mem(mem.vp, sizeof(int), &rq->len, &rq->cap);
						*(rq->e + j) = 0;
						hash_set(rq->h, &np, rq->e + j);
					} else {
						(*(int*)ip) = 0;
					}
					//hash_set(disp->sim->w->chunks, &np, &chunk_reqd_ptr);
				} else {
					(*(int*)ip)++;
				}
			}
}

void display(struct simulation *sim)
{
	int key;
	struct display_container dc;
	struct display disp = {
		.sim = sim,
		.cursor = { 0, 0 },
		.view = { 0, 0 },
		.im = im_normal
	};
	struct timespec tick = { 0, 1000000000 / FPS };
	struct keymap *km, *rkm;
	struct reqd_chunks rq = { .h = hash_init(sizeof(struct point)), .cap = 0, .len = 0, };

	term_setup();
	dc_init(&dc);

	rkm = parse_keymap("defcfg/keymap.ini");
	km = &rkm[disp.im];

	while (sim->run) {
		if ((key = getch()) != ERR)
			if ((km = handle_input(km, key, &disp)) == NULL)
				km = &rkm[disp.im];

		fix_cursor(&dc.root.world->rect, &disp.view, &disp.cursor);
		request_missing_chunks(&rq, &disp, &dc.root.world->rect, &disp.view);

		win_erase();

		draw_infol(dc.root.info.l, &disp.view, &disp.cursor);

		draw_infor(dc.root.info.r, sim->w);

		draw_world(dc.root.world, sim->w, &disp.view);

		if (disp.im == im_select)
			draw_selection(dc.root.world, &disp.cursor);

		win_refresh();

		nanosleep(&tick, NULL);
	}

	term_teardown();
}
