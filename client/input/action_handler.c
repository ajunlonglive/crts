#include "posix.h"

#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "shared/constants/globals.h"
#include "shared/util/log.h"
#include "shared/util/util.h"

static void
set_action_target_int(struct client *cli, long tgt)
{
	switch (cli->next_act.type) {
	case at_build:
		if (tgt < 0) {
			tgt = cli->next_act.tgt + 1;
		}

		tgt %= tile_count;

		while (!gcfg.tiles[tgt].build) {
			tgt = (tgt + 1) % tile_count;
		}

		break;
	case at_carry:
		if (tgt < 0) {
			tgt = cli->next_act.tgt + 1;
		}

		tgt %= ent_type_count;

		while (!gcfg.ents[tgt].holdable) {
			tgt = (tgt + 1) % ent_type_count;
		}

		break;
	default:
		break;
	}

	if (cli->next_act.tgt == tgt) {
		return;
	}

	cli->next_act.tgt = tgt;
	cli->next_act_changed = true;
}

void
set_action_type(struct client *cli)
{
	long id;

	if ((id = client_get_num(cli, 0)) >= action_type_count || id < 0) {
		return;
	} else if (id == cli->next_act.type) {
		return;
	}

	if (cli->keymap_describe) {
		cli_describe(cli, kmc_act_conf, "%s", gcfg.actions[id].name);
	}

	cli->next_act.type = id;
	set_action_target_int(cli, 0);
}

void
set_action_target(struct client *cli)
{
	long tgt = client_get_num(cli, -1);

	if (cli->keymap_describe) {
		switch (cli->next_act.type) {
		case at_build:
			cli_describe(cli, kmc_act_conf, "%-14s", gcfg.tiles[tgt].name);
			break;
		default:
			break;
		}
	}

	set_action_target_int(cli, tgt);
}

void
undo_last_action(struct client *cli)
{
	if (cli->keymap_describe) {
		cli_describe(cli, kmc_act_ctrl, "undo");
		return;
	}

	undo_action(cli);
}

void
exec_action(struct client *cli)
{
	if (cli->keymap_describe) {
		cli_describe(cli, kmc_act_ctrl, "execute current action");
		return;
	}

	if (cli->resize.b) {
		resize_selection_stop(cli);
	}

	cli->next_act.range.pos = point_add(&cli->view, &cli->cursor);
	cli->next_act.workers_requested = client_get_num(cli, 1);

	commit_action(cli);
}

void
resize_selection(struct client *cli)
{
	if (cli->keymap_describe) {
		cli_describe(cli, kmc_resize, "resize selection");
		return;
	}

	if (cli->resize.b) {
		resize_selection_stop(cli);
	} else {
		resize_selection_start(cli);
	}
}
