#include "log.h"
#include "net.h"
#include "window.h"
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>

#include "world.h"

static void fill_window(struct win *win, char fc)
{
	size_t x, y;

	L("filling window %p with %c", win, fc);

	for (x = 0; x < (size_t)win->width; x++)
		for (y = 0; y < (size_t)win->height; y++)
			win_write(win, x, y, fc);
}

static void fill_window_1(struct win *win)
{
	fill_window(win, '_');
}

struct world_view {
	int x;
	int y;
	int z;
};

static struct world *w;
static struct world_view *wv;
static void draw_world(struct win *win)
{
	size_t i;

	for (i = 0; i < w->ecnt; i++)
		if (w->ents[i].x >= wv->x && w->ents[i].y >= wv->y)
			win_write(win, w->ents[i].x, w->ents[i].y, w->ents[i].c);

};

static void net_update_world()
{
	w = world_init();
	struct ent *e = world_spawn(w);
	//world_spawn(e);

	while (1)
		net_demo();
}

static void fill_window_2(struct win *win)
{
	fill_window(win, '#');
}

static void fill_window_3(struct win *win)
{
	fill_window(win, '|');
}

int main(int argc, const char **argv)
{
	setlocale(LC_ALL, "");
	struct win *wworld, *winfo, *winfol, *winfor, *wroot;

	term_setup();
	wroot = win_init(NULL);
	wroot->main_win_pct = 0.8;

	wworld = win_init(wroot);
	wworld->painter = fill_window_1;

	winfo = win_init(wroot);
	winfo->main_win_pct = 0.7;
	winfo->split = 1;

	winfol = win_init(winfo);
	winfol->painter = fill_window_2;

	winfor = win_init(winfo);
	winfor->painter = fill_window_3;

	win_refresh(wroot);

	for (;;)
		sleep(1);

	term_teardown();
	return 0;
}
