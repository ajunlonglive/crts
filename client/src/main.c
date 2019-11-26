#define _DEFAULT_SOURCE

#include "geom.h"
#include "log.h"
#include "net.h"
#include "update.h"
#include "window.h"
#include "world.h"
#include <locale.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

struct state {
	struct {
		struct win *root;
		struct win *_info;
		struct win *infol;
		struct win *infor;
		struct win *world;
	} wins;

	struct point view;

	struct cxinfo *cx;

	struct world *w;

	int run;
};

char default_addr[] = "127.0.0.1";
static struct state gs;

static void draw_infol(struct win *win)
{
	struct point p = { 0, 0 };

	win_printf(win, &p, "simlation running");
}

static void draw_infor(struct win *win)
{
	struct point p = { 0, 0 };

	win_printf(win, &p, "total entities: %d", gs.w->ecnt);
}

static void draw_world(struct win *win)
{
	size_t i;
	struct point np;

	for (i = 0; i < gs.w->ecnt; i++) {
		np = gs.w->ents[i].pos;

		np.x += gs.view.x;
		np.y += gs.view.y;

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

	while (1) {
		ud = queue_pop(q);
		world_apply_update(gs.w, ud);
	}

}

static void gamedisp_init()
{
	term_setup();
	gs.wins.root = win_init(NULL);
	gs.wins.root->main_win_pct = 0.8;

	gs.wins.world = win_init(gs.wins.root);
	gs.wins.world->painter = draw_world;

	gs.wins._info = win_init(gs.wins.root);
	gs.wins._info->main_win_pct = 0.7;
	gs.wins._info->split = 1;

	gs.wins.infol = win_init(gs.wins._info);
	gs.wins.infol->painter = draw_infol;

	gs.wins.infor = win_init(gs.wins._info);
	gs.wins.infor->painter = draw_infor;
}

static void start_threads()
{
	setlocale(LC_ALL, "");
	pthread_t receive_thread, respond_thread, update_world_thread;

	pthread_create(&receive_thread, NULL, thread_receive, (void*)gs.cx);
	pthread_create(&respond_thread, NULL, thread_respond, (void*)gs.cx);
	pthread_create(&update_world_thread, NULL, thread_update_world, (void*)gs.cx->inbound);
}

static void handle_input(int key)
{
	switch (key) {
	case KEY_UP:
		gs.view.y += 4;
		break;
	case KEY_DOWN:
		gs.view.y -= 4;
		break;
	case KEY_LEFT:
		gs.view.x += 4;
		break;
	case KEY_RIGHT:
		gs.view.x -= 4;
		break;
	case 'q':
		gs.run = 0;
		break;
	default:
		break;
	}
}

static void redraw_loop()
{
	struct timespec tick = { 0, 1000000000 / 30 };
	int key;

	while (gs.run) {
		if ((key = getch()) != ERR)
			handle_input(key);

		win_refresh(gs.wins.root);
		nanosleep(&tick, NULL);
	}
}

int main(int argc, const char **argv)
{
	gs.run = 1;

	// initialize world
	gs.w = world_init();

	// connect to server
	gs.cx = argc < 2 ? net_connect(default_addr) : net_connect(argv[1]);
	gs.cx->run = &gs.run;

	// initialize display
	gamedisp_init();

	// start various threads
	start_threads();

	// main thread draw loop
	redraw_loop();

	L("shutting down");

	// cleanup
	term_teardown();

	return 0;
}
