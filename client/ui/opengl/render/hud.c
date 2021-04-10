#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "client/input/action_handler.h"
#include "client/input/handler.h"
#include "client/input/helpers.h"
#include "client/ui/opengl/globals.h"
#include "client/ui/opengl/render/hud.h"
#include "shared/constants/globals.h"
#include "shared/opengl/menu.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

// debug
#include "shared/msgr/transport/rudp.h"

const char *prompt = ":";

static void
render_cmdline(struct opengl_ui_ctx *ctx, struct client *cli)
{
	int32_t i;

	static struct menu_win_ctx win_ctx = { .w = 30, .h = 4, .title = "console" };

	if (menu_win(&ctx->menu, &win_ctx)) {
		for (i = cli->cmdline.history.len - 1; i >= 0; --i) {
			menu_str(&ctx->menu, prompt);
			menu_str(&ctx->menu, cli->cmdline.history.in[i]);

			if (*cli->cmdline.history.out[i]) {
				menu_newline(&ctx->menu);
				menu_str(&ctx->menu, cli->cmdline.history.out[i]);
			}

			menu_newline(&ctx->menu);
		}

		float base_x = ctx->menu.x;
		menu_str(&ctx->menu, prompt);
		menu_str(&ctx->menu, cli->cmdline.cur.buf);

		menu_rect(&ctx->menu, &(struct menu_rect){
			.x = base_x + cli->cmdline.cur.cursor + 1, .y = ctx->menu.y,
			.w = 1, .h = 1
		}, menu_theme_elem_bar_active);

		menu_win_end(&ctx->menu);
	}
}

static void
render_debug_hud(struct opengl_ui_ctx *ctx, struct client *cli)
{
	static struct menu_win_ctx win_ctx = { .title = "debug info" };

	if (menu_win(&ctx->menu, &win_ctx)) {
		menu_printf(&ctx->menu, "t: %.2fms (%.1f fps) | s: %.1f%%, r: %.1f%%",
			ctx->prof.ftime * 1000,
			1 / ctx->prof.ftime,
			100 * ctx->prof.setup / ctx->prof.ftime,
			100 * ctx->prof.render / ctx->prof.ftime);
		menu_newline(&ctx->menu);

		menu_printf(&ctx->menu, "cam: %.2f,%.2f,%.2f p: %.4f y: %.4f",
			cam.pos[0],
			cam.pos[1],
			cam.pos[2],
			cam.pitch,
			cam.yaw
			/* cam.pitch  * (180.0f / PI), */
			/* cam.yaw * (180.0f / PI) */
			);
		menu_newline(&ctx->menu);

		struct point p = point_add(&cli->cursor, &cli->view),
			     q = nearest_chunk(&p),
			     r = point_sub(&p, &q);

		menu_printf(&ctx->menu, "curs:(%d, %d) ck:(%d, %d), rp:(%d, %d), idx:%d",
			p.x, p.y,
			q.x, q.y,
			r.x, r.y,
			r.y * 16 + r.x
			);
		menu_newline(&ctx->menu);

		menu_printf(&ctx->menu, "smo_vc: %ld, chunks: %ld",
			ctx->prof.smo_vert_count,
			ctx->prof.chunk_count
			);
		menu_newline(&ctx->menu);

		if (cli->msgr->transport_impl == msgr_transport_rudp) {
			struct msgr_transport_rudp_ctx *rudp_ctx = cli->msgr->transport_ctx;

			menu_printf(&ctx->menu, "p s:%d/r:%d, m s:%d/r:%d, a:%d",
				rudp_ctx->stats.packets_sent,  rudp_ctx->stats.packets_recvd,
				rudp_ctx->stats.messages_sent, rudp_ctx->stats.messages_recvd,
				rudp_ctx->stats.packets_acked
				);
			menu_newline(&ctx->menu);

			menu_printf(&ctx->menu, "mresent: %d, msize: %d, mcnt: %d",
				rudp_ctx->stats.msg_resent_max,
				rudp_ctx->stats.packet_size_max,
				rudp_ctx->stats.packet_msg_count_max
				);
			menu_newline(&ctx->menu);
		}

		menu_win_end(&ctx->menu);
	}
}

void
render_hud(struct opengl_ui_ctx *ctx, struct client *cli)
{
	menu_begin(&ctx->menu, &ctx->win, ctx->mouse.x, ctx->mouse.y,
		ctx->mouse.buttons & mb_1 && ctx->im_mouse == oim_released);

	if (cli->im == im_cmd) {
		render_cmdline(ctx, cli);
	}

	if (ctx->debug_hud) {
		render_debug_hud(ctx, cli);
	}

	// TODO: add a better way to calculate this
	menu_goto_bottom_right(&ctx->menu);
	ctx->menu.x -= 9;
	ctx->menu.y -= 1;
	menu_rect(&ctx->menu,
		&(struct menu_rect) { .x = ctx->menu.x, .y = ctx->menu.y, .h = 1, .w = 9 },
		menu_theme_elem_bar);
	menu_printf(&ctx->menu, "action: %d", cli->action);

	menu_render(&ctx->menu, &ctx->win);
}
