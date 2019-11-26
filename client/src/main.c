#define _DEFAULT_SOURCE

#include "geom.h"
#include "log.h"
#include "net.h"
#include "update.h"
#include "window.h"
#include "world.h"
#include "state.h"
#include "drawers.h"
#include <locale.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define FPS 30

char default_addr[] = "127.0.0.1";

extern struct state gs;

static void *update_world(void *v)
{
	struct queue *q = v;
	struct update *ud;

	while (1) {
		ud = queue_pop(q);
		world_apply_update(gs.w, ud);
	}

}

static void init_display()
{
	setlocale(LC_ALL, "");
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
	pthread_create(&gs.threads.receive, NULL, (void*)(*net_receive), gs.cx);
	pthread_create(&gs.threads.respond, NULL, (void*)(*net_respond), gs.cx);
	pthread_create(&gs.threads.update, NULL, update_world, (void*)gs.cx->inbound);
}

static void stop_threads()
{
	L("stopping threads");
	pthread_cancel(gs.threads.receive);
	pthread_join(gs.threads.receive, NULL);
	L("canceled receive thread");
	pthread_join(gs.threads.respond, NULL);
	L("joined respond thread");
	pthread_cancel(gs.threads.update);
	pthread_join(gs.threads.update, NULL);
	L("canceled update thread");
}

static void handle_input(int key)
{
	switch (gs.mode) {
	case view_mode_normal:
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
		case 's':
			gs.mode = view_mode_select;
			break;
		case 'q':
			gs.run = 0;
			break;
		default:
			break;
		}
		break;
	case view_mode_select:
		switch (key) {
		case KEY_UP:
			gs.cursor.y -= 4;
			break;
		case KEY_DOWN:
			gs.cursor.y += 4;
			break;
		case KEY_LEFT:
			gs.cursor.x -= 4;
			break;
		case KEY_RIGHT:
			gs.cursor.x += 4;
			break;
		case 'q':
			gs.mode = view_mode_normal;
			break;
		}

		break;
	}
}

static void display()
{
	struct timespec tick = { 0, 1000000000 / FPS };
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
	state_init(&gs);

	// initialize world
	gs.w = world_init();

	// connect to server
	gs.cx = argc < 2 ? net_connect(default_addr) : net_connect(argv[1]);
	gs.cx->run = &gs.run;

	// initialize display
	init_display();

	// start various threads
	start_threads();

	// main redraw loop
	display();

	L("shutting down");

	stop_threads();

	// cleanup
	term_teardown();

	return 0;
}
