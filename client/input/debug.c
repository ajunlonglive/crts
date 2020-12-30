#include "posix.h"

#include "client/client.h"
#include "client/input/debug.h"
#include "shared/pathfind/api.h"
#include "shared/pathfind/preprocess.h"
#include "shared/util/log.h"

void
debug_pathfind_toggle(struct client *cli)
{
	if (cli->keymap_describe) {
		cli_describe(cli, kmc_debug, "enable debug pathfinding mode");
		return;
	}

	if ((cli->debug_path.on = !cli->debug_path.on)) {
		ag_init_components(&cli->sim->w->chunks);

		struct point c = point_add(&cli->view, &cli->cursor);
		cli->debug_path.goal = c;
		L("adding goal @ %d, %d", c.x, c.y);
	}

	cli->sim->changed.chunks = true;
}

void
debug_pathfind_place_point(struct client *cli)
{
	if (cli->keymap_describe) {
		cli_describe(cli, kmc_debug, "place the starting point for the path");
		return;
	}

	if (!cli->debug_path.on) {
		return;
	}

	struct point c = point_add(&cli->view, &cli->cursor);

	if (!hpa_start(&cli->sim->w->chunks, &c, &cli->debug_path.goal, &cli->debug_path.path)) {
		return;
	}

	darr_clear(&cli->debug_path.path_points);

	darr_push(&cli->debug_path.path_points, &c);

	uint32_t i, duplicates = 0;

	while ((hpa_continue(&cli->sim->w->chunks, cli->debug_path.path, &c)) == rs_cont) {
		for (i = 0; i < darr_len(&cli->debug_path.path_points); ++i) {
			struct point *d = darr_get(&cli->debug_path.path_points, i);
			if (points_equal(&c, d)) {
				++duplicates;
			}
		}

		darr_push(&cli->debug_path.path_points, &c);
	}

	L("duplicates in path: %d", duplicates);

	cli->sim->changed.chunks = true;
}
