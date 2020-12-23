#ifndef CLIENT_HIFACE_H
#define CLIENT_HIFACE_H

#include <stdint.h>

#include "client/input/cmdline.h"
#include "client/input/keymap.h"
#include "client/sim.h"
#include "shared/net/net_ctx.h"
#include "shared/sim/action.h"
#include "shared/types/geom.h"

#ifndef NDEBUG
#include "shared/pathfind/api.h"
#endif

#define HF_BUF_LEN 32

struct hiface_buf {
	char buf[HF_BUF_LEN];
	size_t len;
};

struct hiface {
	/* input related buffers */
	struct hiface_buf num;
	struct {
		bool override;
		long val;
	} num_override;
	struct hiface_buf cmd;
	struct cmdline cmdline;

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


	struct {
		struct point cntr, tmpcurs, oldcurs;
		bool b;
	} resize;

	bool display_help;

	struct rectangle viewport;

	/* big pointers */
	struct c_simulation *sim;
	struct net_ctx *nx;
	struct ui_ctx *ui_ctx;

	/* debugging */
#ifndef NDEBUG
	struct {
		bool on;
		uint32_t path;
		struct point goal;
		struct darr path_points;
	} debug_path;
#endif
};

void hiface_init(struct hiface *hf, struct c_simulation *sim);
long hiface_get_num(struct hiface *hif, long def);
void commit_action(struct hiface *hif);
void undo_action(struct hiface *hif);
void override_num_arg(struct hiface *hf, long num);
void hf_describe(struct hiface *hf, enum keymap_category cat, char *desc, ...);
void hiface_reset_input(struct hiface *hf);
void hifb_append_char(struct hiface_buf *hbf, unsigned c);
void check_selection_resize(struct hiface *hf);
void constrain_cursor(struct rectangle *ref, struct point *curs);
void resize_selection_start(struct hiface *hf);
void resize_selection_stop(struct hiface *hf);
void move_viewport(struct hiface *hf, int32_t dx, int32_t dy);
#endif
