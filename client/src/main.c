#define _DEFAULT_SOURCE

#include "update.h"
#include "geom.h"
#include "log.h"
#include "net.h"
#include "window.h"
#include "world.h"
#include <locale.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

static void fill_window(struct win *win, char fc)
{
	struct point p;

	L("filling window %p with %c", win, fc);

	for (p.x = 0; p.x < win->rect.width; p.x++)
		for (p.y = 0; p.y < win->rect.height; p.y++)
			win_write(win, &p, fc);
}

static struct world *w;
//static struct rectangle *wv;

static void draw_world(struct win *win)
{
	size_t i;

	for (i = 0; i < w->ecnt; i++)
		//if (point_in_rect(&w->ents[i].pos, wv))
		win_write(win, &w->ents[i].pos, 'a' + (i % 26));

};

static void fill_window_2(struct win *win)
{
	fill_window(win, '#');
}

static void fill_window_3(struct win *win)
{
	fill_window(win, '|');
}

static void *thread_receive(void *s)
{
	net_receive(s);
	return NULL;
}

static void *thread_respond(void *s)
{
	net_respond(s);
	return NULL;
}

static void *thread_update_world(void *v)
{
	struct queue *q = v;
	struct timespec tick = {
		.tv_sec = 0,
		.tv_nsec = 100000
	};
	struct update *ud;
	struct ent_update *eud;

	while (1) {
		ud = queue_pop(q);
		if (ud == NULL)
			goto sleep;

		L("got an update!");

		eud = ud->update;

		w->ents[eud->id].pos = eud->pos;
		update_destroy(ud);
sleep:
		nanosleep(&tick, NULL);
	}

}

int main(int argc, const char **argv)
{
	setlocale(LC_ALL, "");
	struct win *wworld, *winfo, *winfol, *winfor, *wroot;
	pthread_t receive_thread, respond_thread, update_world_thread;

	w = world_init();
	int i;
	for (i = 0; i < 100; i++)
		world_spawn(w);

	struct server *s = net_connect();
	pthread_create(&receive_thread, NULL, thread_receive, (void*)s);
	pthread_create(&respond_thread, NULL, thread_respond, (void*)s);
	pthread_create(&update_world_thread, NULL, thread_update_world, (void*)s->inbound);

	term_setup();
	wroot = win_init(NULL);
	wroot->main_win_pct = 0.8;

	wworld = win_init(wroot);
	wworld->painter = draw_world;

	winfo = win_init(wroot);
	winfo->main_win_pct = 0.7;
	winfo->split = 1;

	winfol = win_init(winfo);
	winfol->painter = fill_window_2;

	winfor = win_init(winfo);
	winfor->painter = fill_window_3;

	struct timespec tick = {
		.tv_sec = 0,
		//v_nsec = 025000000
		.tv_nsec = 125000000
	};
	while (1) {
		win_refresh(wroot);
		nanosleep(&tick, NULL);
	}

	term_teardown();
	return 0;
}
