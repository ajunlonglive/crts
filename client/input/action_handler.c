#include "posix.h"

#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "shared/constants/globals.h"
#include "shared/util/log.h"
#include "shared/util/util.h"

static void
set_action_target_int(struct hiface *hif, long tgt)
{
	switch (hif->next_act.type) {
	case at_build:
		if (tgt < 0) {
			tgt = hif->next_act.tgt + 1;
		}

		tgt %= tile_count;

		while (!gcfg.tiles[tgt].build) {
			tgt = (tgt + 1) % tile_count;
		}

		break;
	case at_carry:
		if (tgt < 0) {
			tgt = hif->next_act.tgt + 1;
		}

		tgt %= ent_type_count;

		while (!gcfg.ents[tgt].holdable) {
			tgt = (tgt + 1) % ent_type_count;
		}

		break;
	default:
		break;
	}

	if (hif->next_act.tgt == tgt) {
		return;
	}

	hif->next_act.tgt = tgt;
	hif->next_act_changed = true;
}

void
set_action_type(struct hiface *hif)
{
	long id;

	if ((id = hiface_get_num(hif, 0)) >= action_type_count || id < 0) {
		return;
	} else if (id == hif->next_act.type) {
		return;
	}

	if (hif->keymap_describe) {
		hf_describe(hif, kmc_act_conf, "%s", gcfg.actions[id].name);
	}

	hif->next_act.type = id;
	set_action_target_int(hif, 0);
}

void
set_action_target(struct hiface *hif)
{
	long tgt = hiface_get_num(hif, -1);

	if (hif->keymap_describe) {
		switch (hif->next_act.type) {
		case at_build:
			hf_describe(hif, kmc_act_conf, "%-14s", gcfg.tiles[tgt].name);
			break;
		default:
			break;
		}
	}

	set_action_target_int(hif, tgt);
}

void
undo_last_action(struct hiface *hif)
{
	if (hif->keymap_describe) {
		hf_describe(hif, kmc_act_ctrl, "undo");
		return;
	}

	undo_action(hif);
}

void
exec_action(struct hiface *hif)
{
	if (hif->keymap_describe) {
		hf_describe(hif, kmc_act_ctrl, "execute current action");
		return;
	}

	if (hif->resize.b) {
		resize_selection_stop(hif);
	}

	hif->next_act.range.pos = point_add(&hif->view, &hif->cursor);
	hif->next_act.workers_requested = hiface_get_num(hif, 1);

	commit_action(hif);
}

void
resize_selection(struct hiface *hf)
{
	if (hf->keymap_describe) {
		hf_describe(hf, kmc_resize, "resize selection");
		return;
	}

	if (hf->resize.b) {
		resize_selection_stop(hf);
	} else {
		resize_selection_start(hf);
	}
}
