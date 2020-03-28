#ifndef CLIENT_HIFACE_H
#define CLIENT_HIFACE_H

#include <stdint.h>

#include "client/input/keymap.h"
#include "client/sim.h"
#include "shared/net/net_ctx.h"
#include "shared/sim/action.h"
#include "shared/types/geom.h"

struct hiface_buf {
	char buf[16];
	size_t len;
};

struct hiface {
	struct hiface_buf num;
	struct hiface_buf cmd;
	struct simulation *sim;
	struct net_ctx *nx;
	struct point cursor;
	struct point view;
	enum input_mode im;
	struct keymap km[input_mode_count];
	uint32_t redrew_world;

	struct action next_act;
	bool next_act_changed;
	uint8_t action_seq;

	bool center_cursor;
};

struct hiface *hiface_init(struct simulation *sim);
long hiface_get_num(struct hiface *hif, long def);
void commit_action(struct hiface *hif);
void undo_action(struct hiface *hif);
#endif
