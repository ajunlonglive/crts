#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "client/input/helpers.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/render/hud.h"
#include "shared/constants/globals.h"
#include "shared/opengl/render/text.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

// debug
#include "shared/msgr/transport/rudp.h"

struct menu {
	uint8_t indices[64];
	char *desc[64];
	char *trigger[64];
	size_t desc_len[64];
	uint32_t len;
	uint32_t max_len;
	uint32_t pref_len;
	int32_t seli;
	struct keymap *selp;
};

vec4 menu_clr    = { 1, 1,   1, 0.5 };
vec4 sel_clr     = { 1, 1,   0, 0.9 };
vec4 typed_clr   = { 0, 1,   0, 0.9 };
vec4 to_type_clr = { 0, 0.5, 0, 0.9 };

vec4 cmdline_colors[2] = {
	{ 1.0, 1.0, 1.0, 0.8 },
	{ 0.0, 1.0, 0.0, 1.0 }
};

static struct menu completions;

#define SCALE 1

static uint8_t
write_menu(float x, float y, struct client_buf *cmd, struct menu *m, struct client *cli)
{
	size_t len = strlen(cmd->buf);
	size_t numlen = strlen(cli->num.buf);
	uint16_t i = 0, j, row = 0;
	int16_t sel_i = -1;
	float yp, shift;
	enum keymap_category cat = 0;

	gl_printf(x, y, ta_left, "%s mode", input_mode_names[cli->im]);

	for (j = 0; j < m->len; ++j) {
		i = completions.indices[j];

		if (!cat || cat != (uint8_t)completions.desc[i][0]) {
			row += 2;
			cat = completions.desc[i][0];
		} else {
			++row;
		}

		yp = y - row * 1.1f;

		shift = completions.max_len + 1;

		if (numlen) {
			gl_write_string(x + shift, yp, SCALE, typed_clr,
				cli->num.buf);
		}

		if (completions.seli == -1) {
			gl_write_string(x + shift + numlen, yp, SCALE,
				typed_clr, cmd->buf);

			gl_write_string(x + shift + len + numlen, yp, SCALE,
				to_type_clr, &m->trigger[i][len]);
		} else if (completions.seli == i) {
			gl_write_string(x + shift + numlen, yp, SCALE, sel_clr,
				m->trigger[i]);
		} else {
			gl_write_string(x + shift + numlen, yp, SCALE,
				to_type_clr, m->trigger[i]);
		}

		gl_write_string(x + (m->max_len - m->desc_len[i]), yp, SCALE,
			completions.seli == i ? sel_clr : menu_clr,
			&m->desc[i][1]);
	}

	return sel_i;
}

static int
compare_completion_menu_item(const void *_a, const void *_b)
{
	uint8_t a = *(uint8_t *)_a, b = *(uint8_t *)_b;

	return strcmp(completions.desc[a], completions.desc[b]);
}

static void
add_completion_menu_item(void *_ctx, struct keymap *km)
{
	//struct opengl_ui_ctx *ctx = _ctx;

	if (*km->desc == '\0') {
		return;
	}

	size_t len;
	if ((len = strlen(km->desc)) > completions.max_len) {
		completions.max_len = len;
	}

	if (km == completions.selp) {
		completions.seli = completions.len;
	}

	completions.indices[completions.len] = completions.len;

	completions.trigger[completions.len] = km->trigger;

	completions.desc[completions.len] = km->desc;

	completions.desc_len[completions.len] = len;

	++completions.len;
}

static void
render_completions(float x, float y, struct opengl_ui_ctx *ctx, struct client *cli)
{
	static bool regen_menu = true;
	struct client_buf cmd = cli->cmd;

	if (regen_menu || cli->changed.input) {
		regen_menu = false;
		completions.seli = -1;
		completions.selp = NULL;
		completions.len = 0;
		completions.pref_len = 0;
		completions.max_len = 0;

		if (cmd.len == 0 && ctx->last_key) {
			struct keymap *tmp = ctx->ckm;
			struct im;
			ctx->ckm = ctx->okm;

			if (!(completions.selp = &ctx->ckm->map[(uint8_t)ctx->last_key])->map) {
				clib_append_char(&cmd, ctx->last_key);
			} else {
				completions.selp = NULL;
				ctx->ckm = tmp;
			}

			ctx->last_key = 0;
			regen_menu = true;
		}

		enum input_mode im = cli->im;
		cli->im = ctx->oim;
		describe_completions(cli, ctx->ckm, ctx, add_completion_menu_item);
		cli->im = im;

		qsort(completions.indices, completions.len, sizeof(uint8_t),
			compare_completion_menu_item);
	}

	write_menu(x, y, &cmd, &completions, cli);
}

static void
render_cmdbuf(float x, float y, size_t prompt_len, const char *prompt,
	const char *line, int32_t cursor)
{
	size_t i, len = strlen(line);

	float sx, sy;
	screen_coords_to_text_coords(x, y, &sx, &sy);
	gl_write_string(x, y, 0.0, cmdline_colors[0], prompt);

	for (i = 0; i < len; ++i) {
		gl_write_char(sx + i + prompt_len, sy,
			cmdline_colors[cursor >= 0 ? i == (uint32_t)cursor : 0],
			line[i]);
	}
}

static void
render_cmdline(struct opengl_ui_ctx *ctx, struct client *cli)
{
	uint32_t i, off = 1;
	bool output;
	static const char *prompt[] = { ":", " " };
	size_t prompt_len[] = { strlen(prompt[0]), strlen(prompt[1]) };

	for (i = 0; i < cli->cmdline.history.len; ++i) {
		output = *cli->cmdline.history.out[i] != 0;

		render_cmdbuf(0, off + output, prompt_len[0], prompt[0],
			cli->cmdline.history.in[i], -1);

		if (output) {
			render_cmdbuf(0, off, prompt_len[1], prompt[1],
				cli->cmdline.history.out[i], -1);
		}

		off += 1 + output;
	}

	render_cmdbuf(0, 0, prompt_len[0], prompt[0], cli->cmdline.cur.buf,
		cli->cmdline.cur.cursor);

	if (cli->cmdline.cur.cursor == cli->cmdline.cur.len) {
		gl_write_char(0 + cli->cmdline.cur.len + prompt_len[0], 0,
			cmdline_colors[1], ' ');
	}
}

void
render_hud(struct opengl_ui_ctx *ctx, struct client *cli)
{
	const char *act_tgt_nme;
	float sx, sy;

	screen_coords_to_text_coords(2, -3, &sx, &sy);

	switch (cli->next_act.type) {
	case at_build:
		act_tgt_nme = gcfg.tiles[cli->next_act.tgt].name;
		break;
	default:
		act_tgt_nme = "";
		break;
	}

	screen_coords_to_text_coords(-1, 0, &sx, &sy);
	gl_printf(sx, sy, ta_right, "action: %s %s",
		gcfg.actions[cli->next_act.type].name,
		act_tgt_nme);

	if (cli->im == im_cmd) {
		render_cmdline(ctx, cli);
	}

	if (cli->state & csf_display_help) {
		screen_coords_to_text_coords(0, -1, &sx, &sy);
		render_completions(sx, sy, ctx, cli);
	}
}

/* must be called AFTER render_hud */
void
render_debug_hud(struct opengl_ui_ctx *ctx, struct client *cli)
{
	float sx, sy;
	screen_coords_to_text_coords(0, -1, &sx, &sy);
	gl_printf(sx, sy, ta_left, "t: %.2fms (%.1f fps) | s: %.1f%%, r: %.1f%%",
		ctx->prof.ftime * 1000,
		1 / ctx->prof.ftime,
		100 * ctx->prof.setup / ctx->prof.ftime,
		100 * ctx->prof.render / ctx->prof.ftime);

	screen_coords_to_text_coords(0, -2, &sx, &sy);
	gl_printf(sx, sy, ta_left, "cam: %.2f,%.2f,%.2f p: %.4f y: %.4f",
		cam.pos[0],
		cam.pos[1],
		cam.pos[2],
		cam.pitch,
		cam.yaw
		/* cam.pitch  * (180.0f / PI), */
		/* cam.yaw * (180.0f / PI) */
		);

	struct point p = point_add(&cli->cursor, &cli->view),
		     q = nearest_chunk(&p),
		     r = point_sub(&p, &q);

	screen_coords_to_text_coords(0, -3, &sx, &sy);
	gl_printf(sx, sy, ta_left, "curs:(%d, %d) ck:(%d, %d), rp:(%d, %d), idx:%d",
		p.x, p.y,
		q.x, q.y,
		r.x, r.y,
		r.y * 16 + r.x
		);

	screen_coords_to_text_coords(0, -4, &sx, &sy);
	gl_printf(sx, sy, ta_left, "smo_vc: %ld, chunks: %ld",
		ctx->prof.smo_vert_count,
		ctx->prof.chunk_count
		);

	if (cli->msgr->transport_impl == msgr_transport_rudp) {
		struct msgr_transport_rudp_ctx *ctx = cli->msgr->transport_ctx;

		screen_coords_to_text_coords(0, -5, &sx, &sy);
		gl_printf(sx, sy, ta_left, "p s:%d/r:%d, m s:%d/r:%d, a:%d",
			ctx->stats.packets_sent, ctx->stats.packets_recvd,
			ctx->stats.messages_sent, ctx->stats.messages_recvd,
			ctx->stats.packets_acked
			);

		screen_coords_to_text_coords(0, -6, &sx, &sy);
		gl_printf(sx, sy, ta_left, "mresent: %d, msize: %d, mcnt: %d",
			ctx->stats.msg_resent_max,
			ctx->stats.packet_size_max,
			ctx->stats.packet_msg_count_max
			);
	}
}
