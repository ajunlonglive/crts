#ifndef CLIENT_HIFACE_H
#define CLIENT_HIFACE_H

#include <stdint.h>

#include "client/input/keymap.h"
#include "client/sim.h"
#include "shared/net/net_ctx.h"
#include "shared/sim/action.h"
#include "shared/types/geom.h"

#define HF_BUF_LEN 256
struct hiface_buf {
	size_t len;
	char buf[HF_BUF_LEN];
};

struct hiface {
	struct hiface_buf num;
	struct {
		bool override;
		long val;
	} num_override;
	struct hiface_buf cmd;
	struct c_simulation *sim;
	struct net_ctx *nx;
	struct point cursor;
	struct point view;
	enum input_mode im;
	struct keymap km[input_mode_count];
	uint32_t redrew_world;

	struct action next_act;
	bool next_act_changed;
	uint8_t action_seq;

	bool keymap_describe;
	char description[KEYMAP_DESC_LEN];
	size_t desc_len;
	bool input_changed;

	bool center_cursor;
	bool display_help;
};

struct hiface *hiface_init(struct c_simulation *sim);
long hiface_get_num(struct hiface *hif, long def);
void commit_action(struct hiface *hif);
void undo_action(struct hiface *hif);
void override_num_arg(struct hiface *hf, long num);
void hf_describe(struct hiface *hf, enum keymap_category cat, char *desc, ...);
void hiface_reset_input(struct hiface *hf);
void hifb_append_char(struct hiface_buf *hbf, unsigned c);
#endif
