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

struct gamedisp {
	struct win *root;
	struct win *_info;
	struct win *infol;
	struct win *infor;
	struct win *world;

	struct point wview;
};

char default_addr[] = "127.0.0.1";
static struct world *w;
static struct gamedisp gd;

static void fill_window(struct win *win, char fc)
{
	struct point p;

	for (p.x = 0; p.x < win->rect.width; p.x++)
		for (p.y = 0; p.y < win->rect.height; p.y++)
			win_write(win, &p, fc);
}

static void fill_window_2(struct win *win)
{
	fill_window(win, '#');
}

static void fill_window_3(struct win *win)
{
	fill_window(win, '|');
}

static void draw_world(struct win *win)
{
	size_t i;
	struct point np;

	for (i = 0; i < w->ecnt; i++) {
		np = w->ents[i].pos;

		np.x += gd.wview.x;
		np.y += gd.wview.y;

		win_write(win, &np, '@');
	}

};

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
	struct update *ud;
	struct ent_update *eud;

	while (1) {
		ud = queue_pop(q);
		eud = ud->update;

		L("updating position of end %d", eud->id);
		w->ents[eud->id].pos = eud->pos;
		update_destroy(ud);
	}

}

static void gamedisp_init()
{
	term_setup();
	gd.root = win_init(NULL);
	gd.root->main_win_pct = 0.8;

	gd.world = win_init(gd.root);
	gd.world->painter = draw_world;

	gd._info = win_init(gd.root);
	gd._info->main_win_pct = 0.7;
	gd._info->split = 1;

	gd.infol = win_init(gd._info);
	gd.infol->painter = fill_window_2;

	gd.infor = win_init(gd._info);
	gd.infor->painter = fill_window_3;
}

static void start_threads(struct server *s)
{
	setlocale(LC_ALL, "");
	pthread_t receive_thread, respond_thread, update_world_thread;

	pthread_create(&receive_thread, NULL, thread_receive, (void*)s);
	pthread_create(&respond_thread, NULL, thread_respond, (void*)s);
	pthread_create(&update_world_thread, NULL, thread_update_world, (void*)s->inbound);
}

static void handle_input(int key)
{
	switch (key) {
	case KEY_UP:
		gd.wview.y += 4;
		break;
	case KEY_DOWN:
		gd.wview.y -= 4;
		break;
	case KEY_LEFT:
		gd.wview.x += 4;
		break;
	case KEY_RIGHT:
		gd.wview.x -= 4;
		break;
	default:
		break;
	}
}

static void redraw_loop()
{
	struct timespec tick = { 0, 1000000000 / 30 };
	int key;

	while (1) {
		if ((key = getch()) != ERR)
			handle_input(key);

		win_refresh(gd.root);
		nanosleep(&tick, NULL);
	}
}

int main(int argc, const char **argv)
{
	int i;
	struct server *s;

	// initialize world
	w = world_init();

	// spawn creatures
	for (i = 0; i < 1000; i++)
		world_spawn(w);
	// connect to server
	s = argc < 2 ? net_connect(default_addr) : net_connect(argv[1]);

	// initialize display
	gamedisp_init();

	// start various threads
	start_threads(s);

	// main thread draw loop
	redraw_loop();

	// cleanup (never reached)
	term_teardown();

	return 0;
}
