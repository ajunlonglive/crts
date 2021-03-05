#include "posix.h"

#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "client/input/helpers.h"
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
	cli->changed.next_act = true;
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

	uint8_t na;
	struct action *act;

	if (cli->action_history_len == 0) {
		return;
	}

	--cli->action_history_len;

	na = cli->action_history_order[cli->action_history_len];
	act = &cli->action_history[na];
	act->type = at_none;

	cli->changed.next_act = true;

	struct msg_action msg = {
		.mt = amt_del,
		.id = act->id, /* TODO we only need the id on del? */
	};

	msgr_queue(cli->msgr, mt_action, &msg, 0, priority_normal);
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

	if (cli->next_act.type == at_none) {
		return;
	}

	cli->next_act.id = (cli->action_seq++) % ACTION_HISTORY_SIZE;
	cli->action_history[cli->next_act.id] = cli->next_act;

	cli->action_history_order[cli->action_history_len] = cli->next_act.id;

	/* TODO: relying on uint8_t overflow to keep index in bounds */
	++cli->action_history_len;

	struct msg_action msg = {
		.mt = amt_add,
		.id = cli->next_act.id,
		.dat = {
			.add = {
				.tgt = cli->next_act.tgt,
				.type = cli->next_act.type,
				.range = cli->next_act.range
			},
		}
	};

	msgr_queue(cli->msgr, mt_action, &msg, 0, priority_normal);
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
