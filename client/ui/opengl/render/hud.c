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

static vec4 cmdline_colors[2] = {
	{ 1.0, 1.0, 1.0, 0.8 },
	{ 0.0, 1.0, 0.0, 1.0 }
};

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
	float sx, sy;

	screen_coords_to_text_coords(2, -3, &sx, &sy);

	screen_coords_to_text_coords(-1, 0, &sx, &sy);
	gl_printf(sx, sy, ta_right, "action: %d",
		cli->action);

	if (cli->im == im_cmd) {
		render_cmdline(ctx, cli);
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
