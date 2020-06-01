#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "shared/constants/globals.h"
#include "shared/messaging/client_message.h"
#include "shared/util/log.h"

/* When setting this number as the action type, the bldg_rotate bit will be
 * flipped in the current target instad */
#define MAGIC_ROTATE_NUMBER 64
#define MAX_RANGE 64

static void
set_action_target_int(struct hiface *hif, long tgt)
{
	switch (hif->next_act.type) {
	case at_build:
		if (tgt < 0) {
			tgt = hif->next_act.tgt + 2;
		} else if (tgt == MAGIC_ROTATE_NUMBER) {
			tgt = hif->next_act.tgt ^ bldg_rotate;
		}

		tgt %= buildings_count;
		break;
	case at_harvest:
		if (tgt < 0) {
			tgt = hif->next_act.tgt + 1;
		}

		tgt %= tile_count;

		while (!gcfg.tiles[tgt].hardness) {
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
		return;
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
	}

	hif->next_act.type = id;
	set_action_target_int(hif, 0);
	hif->next_act_changed = true;
}

int
action_radius_clamp(int r)
{
	if (r > MAX_RANGE) {
		r = MAX_RANGE;
	} else if (r < 1) {
		r = 1;
	}

	return r;
}

void
set_action_radius(struct hiface *hif)
{
	long r = hiface_get_num(hif, 1);

	hif->next_act.range.r = action_radius_clamp(r);
	hif->next_act_changed = true;
}

void
action_radius_expand(struct hiface *hif)
{
	long r = hiface_get_num(hif, 1);

	hif->next_act.range.r = action_radius_clamp(hif->next_act.range.r + r);
	hif->next_act_changed = true;
}

void
action_radius_shrink(struct hiface *hif)
{
	long r = hiface_get_num(hif, 1);

	hif->next_act.range.r = action_radius_clamp(hif->next_act.range.r - r);
	hif->next_act_changed = true;
}

void
set_action_source(struct hiface *hif)
{
	long r = hiface_get_num(hif, 1);

	hif->next_act.source.r = action_radius_clamp(r);
	hif->next_act.source.center = point_add(&hif->view, &hif->cursor);
	hif->next_act_changed = true;
}

void
swap_cursor_with_source(struct hiface *hif)
{
	struct circle tmp;

	tmp = hif->next_act.source;

	hif->next_act.source.r = hif->next_act.range.r;
	hif->next_act.source.center = point_add(&hif->view, &hif->cursor);

	hif->next_act.range.r = tmp.r;
	hif->cursor = point_sub(&tmp.center, &hif->view);

	hif->next_act_changed = true;
}

void
set_action_target(struct hiface *hif)
{
	long tgt = hiface_get_num(hif, -1);

	set_action_target_int(hif, tgt);
}

long
read_action_target_from_world(struct hiface *hif)
{
	struct point cp, rp = point_add(&hif->cursor, &hif->view);
	struct chunk *ck;
	cp = nearest_chunk(&rp);

	if (!(ck = hdarr_get(hif->sim->w->chunks->hd, &cp))) {
		return 0;
	}

	cp = point_sub(&rp, &ck->pos);
	L("read %s", gcfg.tiles[ck->tiles[cp.x][cp.y]].name);

	return ck->tiles[cp.x][cp.y];
}

void
read_action_target(struct hiface *hif)
{
	long tgt;

	switch (hif->next_act.type) {
	case at_harvest:
		tgt = read_action_target_from_world(hif);
		break;
	default:
		tgt = 0;
		break;
	}

	set_action_target_int(hif, tgt);
}

void
toggle_action_flag(struct hiface *hif)
{
	long r = hiface_get_num(hif, -1);

	if (r < 0 || r >= action_flags_count) {
		return;
	}

	hif->next_act.flags ^= 1 << r;
}

void
undo_last_action(struct hiface *hif)
{
	undo_action(hif);
}

void
exec_action(struct hiface *hif)
{
	hif->next_act.range.center = point_add(&hif->view, &hif->cursor);
	hif->next_act.workers_requested = hiface_get_num(hif, 1);

	commit_action(hif);

	hif->next_act.flags = 0;
}
