#include "log.h"
#include "net.h"
#include "geom.h"
#include "window.h"
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>

#include "world.h"

static void fill_window(struct win *win, char fc)
{
	struct point p;

	L("filling window %p with %c", win, fc);

	for (p.x = 0; p.x < win->rect.width; p.x++)
		for (p.y = 0; p.y < win->rect.height; p.y++)
			win_write(win, &p, fc);
}

static void fill_window_1(struct win *win)
{
	fill_window(win, '_');
}

static struct world *w;
static struct rectangle *wv;
static void draw_world(struct win *win)
{
	size_t i;

	for (i = 0; i < w->ecnt; i++)
		if (point_in_rect(&w->ents[i].pos, wv))
			win_write(win, &w->ents[i].pos, w->ents[i].c);

};

static void net_update_world()
{
	w = world_init();
	//struct ent *e = world_spawn(w);
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
