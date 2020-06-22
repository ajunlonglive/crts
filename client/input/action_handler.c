#include "posix.h"

#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "shared/constants/globals.h"
#include "shared/messaging/client_message.h"
#include "shared/util/log.h"

/* When setting this number as the action type, the bldg_rotate bit will be
 * flipped in the current target instad */
#define MAGIC_ROTATE_NUMBER 64
#define MAX_HEIGHT 64
#define MAX_WIDTH 64

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
	} else if (id == hif->next_act.type) {
		return;
	}

	hif->next_act.type = id;
	set_action_target_int(hif, 0);
	hif->next_act_changed = true;
}

static int32_t
clamp(int32_t v, int32_t min, int32_t max)
{
	if (v > max) {
		return max;
	} else if (v < min) {
		return min;
	} else {
		return v;
	}
}

void
set_action_height(struct hiface *hif)
{
	hif->next_act.range.height = clamp(hiface_get_num(hif, 1), 1, MAX_HEIGHT);
	hif->next_act_changed = true;
}

void
action_height_grow(struct hiface *hif)
{
	hif->next_act.range.height = clamp(
		hif->next_act.range.height + hiface_get_num(hif, 1),
		1, MAX_HEIGHT);
	hif->next_act_changed = true;
}

void
action_height_shrink(struct hiface *hif)
{
	hif->next_act.range.height = clamp(
		hif->next_act.range.height - hiface_get_num(hif, 1),
		1, MAX_HEIGHT);
	hif->next_act_changed = true;
}

void
set_action_width(struct hiface *hif)
{
	hif->next_act.range.width = clamp(hiface_get_num(hif, 1), 1, MAX_WIDTH);
	hif->next_act_changed = true;
}

void
action_width_grow(struct hiface *hif)
{
	hif->next_act.range.width = clamp(
		hif->next_act.range.width + hiface_get_num(hif, 1),
		1, MAX_WIDTH);
	hif->next_act_changed = true;
}

void
action_width_shrink(struct hiface *hif)
{
	hif->next_act.range.width = clamp(
		hif->next_act.range.width - hiface_get_num(hif, 1),
		1, MAX_WIDTH);

	hif->next_act_changed = true;
}

void
action_rect_rotate(struct hiface *hif)
{
	int tmp = hif->next_act.range.width;

	hif->next_act.range.width = clamp(hif->next_act.range.height, 1,
		MAX_WIDTH);

	hif->next_act.range.height = clamp(tmp, 1, MAX_WIDTH);

	hif->next_act_changed = true;
}

void
swap_cursor_with_source(struct hiface *hif)
{
	struct rectangle tmp;

	tmp = hif->next_act.source;

	hif->next_act.source.width = hif->next_act.range.width;
	hif->next_act.source.pos = point_add(&hif->view, &hif->cursor);

	hif->next_act.range.width = tmp.width;
	hif->cursor = point_sub(&tmp.pos, &hif->view);

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
	hif->next_act.range.pos = point_add(&hif->view, &hif->cursor);
	hif->next_act.workers_requested = hiface_get_num(hif, 1);

	L("setting harvest target, (%d, %d), %dx%d",
		hif->next_act.range.pos.x,
		hif->next_act.range.pos.y,
		hif->next_act.range.height,
		hif->next_act.range.width
		);
	commit_action(hif);

	hif->next_act.flags = 0;
}
