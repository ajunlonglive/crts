#ifndef CLIENT_HIFACE_H
#define CLIENT_HIFACE_H

#include <stdint.h>

#include "client/input/cmdline.h"
#include "client/input/keymap.h"
#include "client/sim.h"
#include "shared/msgr/msgr.h"
#include "shared/sim/action.h"
#include "shared/types/geom.h"
/* #include "shared/net/net_ctx.h" */

#ifndef NDEBUG
#include "shared/pathfind/api.h"
#endif

#define HF_BUF_LEN 32

struct client_buf {
	char buf[HF_BUF_LEN];
	size_t len;
};

struct client {
	/* input related buffers */
	struct client_buf num;
	struct {
		bool override;
		long val;
	} num_override;
	struct client_buf cmd;
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
	/* struct net_ctx *nx; */
	struct msgr *msgr;
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

void client_init(struct client *cli, struct c_simulation *sim);
long client_get_num(struct client *cli, long def);
void commit_action(struct client *cli);
void undo_action(struct client *cli);
void override_num_arg(struct client *cli, long num);
void cli_describe(struct client *cli, enum keymap_category cat, char *desc, ...);
void client_reset_input(struct client *cli);
void clib_append_char(struct client_buf *hbf, unsigned c);
void check_selection_resize(struct client *cli);
void constrain_cursor(struct rectangle *ref, struct point *curs);
void resize_selection_start(struct client *cli);
void resize_selection_stop(struct client *cli);
void move_viewport(struct client *cli, int32_t dx, int32_t dy);
#endif
