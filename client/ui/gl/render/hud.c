#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "client/client.h"
#include "client/ui/gl/globals.h"
#include "client/ui/gl/render/hud.h"
#include "shared/constants/globals.h"
#include "shared/input/mouse.h"
#include "shared/types/darr.h"
#include "shared/ui/gl/menu.h"
#include "shared/util/log.h"
#include "tracy.h"
#include "version.h"

// debug
#include "shared/msgr/transport/rudp.h"

const char *prompt = ":";

static void
render_cmdline(struct gl_ui_ctx *ctx, struct client *cli)
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
			.x = base_x + cli->cmdline.cur.cursor + 1, .y = ctx->menu.y + 0.7,
			.w = 1, .h = 0.3
		}, menu_theme_elem_bar_accent);

		menu_win_end(&ctx->menu);
	}
}

static void
render_debug_hud(struct gl_ui_ctx *ctx, struct client *cli)
{
	static struct menu_win_ctx win_ctx = { .title = "debug info" };

	if (menu_win(&ctx->menu, &win_ctx)) {
		float fps = 1.0 / cli->prof.client_tick.avg;

		menu_printf(&ctx->menu, "fps: %.1f (%.1f, %.1f) | sim: %.1f ",
			fps,
			ctx->prof.setup.avg * 1000,
			ctx->prof.render.avg * 1000,
			cli->prof.server_fps
			);
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

		struct chunk *ck = get_chunk(&cli->world->chunks, &q);
		menu_printf(&ctx->menu, "tile: %s, height: %f",
			gcfg.tiles[ck->tiles[r.x][r.y]].name,
			ck->heights[r.x][r.y]
			);
		menu_newline(&ctx->menu);

		menu_printf(&ctx->menu, "smo_vc: %ld, chunks: %ld, ents: %ld",
			ctx->prof.smo_vert_count,
			ctx->prof.chunk_count,
			hdarr_len(&ctx->cli->world->ents)
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

static void
render_pause_menu(struct gl_ui_ctx *ctx, struct client *cli)
{

	const float w = ctx->menu.gl_win->sc_width / ctx->menu.scale;
	const float h = ctx->menu.gl_win->sc_height / ctx->menu.scale;
	const float col1 = w / 4, col2 = w / 2;
	/* L(log_misc, "%fx%f", h, w); */

	menu_rect(&ctx->menu,
		&(struct menu_rect) { .x = 0, .y = 0, .h = h, .w = w },
		menu_theme_elem_win);

	ctx->menu.x = col1; ctx->menu.y = 4;
	menu_printf(&ctx->menu, "crts v%s %s", crts_version.version, crts_version.vcs_tag);
	ctx->menu.x = col1; ctx->menu.y += 2;

	{
		static float vol = 1.0f;
		menu_printf(&ctx->menu, "volume: %3.0f%%", vol * 100.0f);
		ctx->menu.x = col2;

		static struct menu_slider_ctx slider = { .min = 0.0f, .max = 1.0f };
		slider.w = col1;
		menu_slider(&ctx->menu, &slider, &vol);

		ctx->menu.x = col1; ctx->menu.y += 1.5;
	}

	{
		menu_printf(&ctx->menu, "ui scale: %5.1f", ctx->menu.scale);
		ctx->menu.x = col2;

		enum menu_button_flags flags;

		flags = ctx->menu.scale < 30.0f ? 0 : menu_button_flag_disabled;

		if (menu_button(&ctx->menu, "  +  ", flags)) {
			menu_set_scale(&ctx->menu, ctx->menu.scale + 1);
		}
		ctx->menu.x += 1;

		flags = ctx->menu.scale > 15.0f ? 0 : menu_button_flag_disabled;

		if (menu_button(&ctx->menu, "  -  ", flags)) {
			menu_set_scale(&ctx->menu, ctx->menu.scale - 1);
		}

		ctx->menu.x = col1; ctx->menu.y += 1.5;
	}

	{
		ctx->menu.x = col1; ctx->menu.y += 1.5;

		if (menu_button(&ctx->menu, "resume", 0)) {
			cli->state &= ~csf_paused;
		}

		ctx->menu.x = col1; ctx->menu.y += 1.5;

		if (menu_button(&ctx->menu, " quit ", 0)) {
			cli->run = false;
		}

		ctx->menu.x = col1; ctx->menu.y += 1.5;
	}
}

void
render_hud(struct gl_ui_ctx *ctx, struct client *cli)
{
	TracyCZoneAutoS;

	menu_begin(&ctx->menu, ctx->win, ctx->win->mouse.x, ctx->win->mouse.y,
		ctx->win->mouse.buttons & mb_1 && ctx->cursor_enabled);

	if (cli->im == im_cmd) {
		render_cmdline(ctx, cli);
	}

	if (ctx->debug_hud) {
		render_debug_hud(ctx, cli);
	}

	if (cli->state & csf_paused) {
		render_pause_menu(ctx, cli);
	}

	float width = ctx->menu.gl_win->sc_width / ctx->menu.scale;
	float ratio = (float)ctx->stats.friendly_ent_count / (float)ctx->stats.live_ent_count;

	menu_rect_printf(&ctx->menu, &(struct menu_rect) {
		.x = 0,
		.y = (ctx->menu.gl_win->sc_height / ctx->menu.scale) - 2,
		.h = 2, .w = ratio * width
	}, menu_theme_elem_bar, menu_align_left, "%d", ctx->stats.friendly_ent_count);

	menu_rect_printf(&ctx->menu, &(struct menu_rect) {
		.x = ratio * width,
		.y = (ctx->menu.gl_win->sc_height / ctx->menu.scale) - 2,
		.h = 2, .w = width - (ratio * width)
	}, menu_theme_elem_bar_accent, menu_align_right, "%d",
		ctx->stats.live_ent_count - ctx->stats.friendly_ent_count);

	menu_render(&ctx->menu, ctx->win);

	TracyCZoneAutoE;
}
