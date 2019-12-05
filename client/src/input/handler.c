#include <stdlib.h>

#include "../sim.h"
#include "../display.h"
#include "handler.h"
#include "util/log.h"

#define MOVE_AMNT 5;

static void view_up(void *d)
{
	((struct display *)d)->view.y -= MOVE_AMNT;
}

static void view_down(void *d)
{
	((struct display *)d)->view.y += MOVE_AMNT;
}

static void view_left(void *d)
{
	((struct display *)d)->view.x -= MOVE_AMNT;
}

static void view_right(void *d)
{
	((struct display *)d)->view.x += MOVE_AMNT;
}

static void end_simulation(void *disp)
{
	((struct display *)disp)->sim->run = 0;
}

static void do_nothing(void *_)
{
}

static void (*const kc_func[KEY_COMMANDS])(void *) = {
	[kc_none]                 = do_nothing,
	[kc_invalid]              = do_nothing,
	[kc_view_up]              = view_up,
	[kc_view_down]            = view_down,
	[kc_view_left]            = view_left,
	[kc_view_right]           = view_right,
	[kc_enter_selection_mode] = do_nothing,
	[kc_quit]                 = end_simulation,
	[kc_cursor_left]          = do_nothing,
	[kc_cursor_down]          = do_nothing,
	[kc_cursor_up]            = do_nothing,
	[kc_cursor_right]         = do_nothing,
	[kc_create_move_action]   = do_nothing,
};

struct keymap *handle_input(struct keymap *km, unsigned k, struct display *sim)
{
	km = &km->map[k];

	L("got keymap for %c @ %p", k, km);

	if (km->map == NULL) {
		L("key is final, executing command");
		kc_func[km->cmd](sim);
		km = NULL;
	}

	return km;
}

