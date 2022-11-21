#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "client/client.h"
#include "client/ui/gl/colors.h"
#include "client/ui/gl/globals.h"
#include "client/ui/gl/render/hud.h"
#include "client/ui/gl/render/settings_menu.h"
#include "shared/constants/globals.h"
#include "shared/input/keyboard.h"
#include "shared/input/mouse.h"
#include "shared/types/darr.h"
#include "shared/ui/gl/menu.h"
#include "shared/ui/gl/render/shapes.h"
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

	if (menu_win(&win_ctx)) {
		for (i = cli->cmdline.history.len - 1; i >= 0; --i) {
			menu_str(prompt);
			menu_str(cli->cmdline.history.in[i]);

			if (*cli->cmdline.history.out[i]) {
				menu_newline();
				menu_str(cli->cmdline.history.out[i]);
			}

			menu_newline();
		}

		float base_x = menu.x;
		menu_str(prompt);
		menu_str(cli->cmdline.cur.buf);

		menu_rect(&(struct menu_rect){
			.x = base_x + cli->cmdline.cur.cursor + 1, .y = menu.y + 0.7,
			.w = 1, .h = 0.3
		}, menu_theme_elem_bar_accent);

		menu_win_end();
	}
}

static void
render_debug_hud(struct gl_ui_ctx *ctx, struct client *cli)
{
	static struct menu_win_ctx win_ctx = { .title = "debug info" };

	if (menu_win(&win_ctx)) {
		float fps = 1.0 / cli->prof.client_tick.avg;

		menu_printf("fps: %.1f (%.1f, %.1f) | sim: %.1f ",
			fps,
			ctx->prof.setup.avg * 1000,
			ctx->prof.render.avg * 1000,
			cli->prof.server_fps
			);
		menu_newline();

		menu_printf("cam: %.2f,%.2f,%.2f p: %.4f y: %.4f",
			cam.pos[0],
			cam.pos[1],
			cam.pos[2],
			cam.pitch,
			cam.yaw
			/* cam.pitch  * (180.0f / PI), */
			/* cam.yaw * (180.0f / PI) */
			);
		menu_newline();

		struct point q = nearest_chunk(&cli->cursor),
			     r = point_sub(&cli->cursor, &q);

		menu_printf("curs:(%d, %d, %d) ck:(%d, %d), rp:(%d, %d), idx:%d",
			cli->cursor.x, cli->cursor.y, (int32_t)cli->cursor_z,
			q.x, q.y,
			r.x, r.y,
			r.y * 16 + r.x
			);
		menu_newline();

		struct chunk *ck = get_chunk(&cli->world->chunks, &q);
		menu_printf("tile: %s, height: %f",
			gcfg.tiles[ck->tiles[r.x][r.y]].name,
			ck->heights[r.x][r.y]
			);
		menu_newline();

		menu_printf("smo_vc: %ld, chunks: %ld, ents: %ld",
			ctx->prof.smo_vert_count,
			ctx->prof.chunk_count,
			hdarr_len(&cli->world->ents)
			);
		menu_newline();

		if (cli->msgr->transport_impl == msgr_transport_rudp) {
			struct msgr_transport_rudp_ctx *rudp_ctx = cli->msgr->transport_ctx;

			menu_printf("p s:%d/r:%d, m s:%d/r:%d, a:%d",
				rudp_ctx->stats.packets_sent,  rudp_ctx->stats.packets_recvd,
				rudp_ctx->stats.messages_sent, rudp_ctx->stats.messages_recvd,
				rudp_ctx->stats.packets_acked
				);
			menu_newline();

			menu_printf("mresent: %d, msize: %d, mcnt: %d",
				rudp_ctx->stats.msg_resent_max,
				rudp_ctx->stats.packet_size_max,
				rudp_ctx->stats.packet_msg_count_max
				);
			menu_newline();
		}

		menu_win_end();
	}
}

static void
render_pause_menu(struct gl_ui_ctx *ctx, struct client *cli)
{
	const float w = menu.gl_win->sc_width / menu.scale;
	const float h = menu.gl_win->sc_height / menu.scale;

	menu_rect(&(struct menu_rect) { .x = 0, .y = 0, .h = h, .w = w },
		menu_theme_elem_win);

	menu.y = 5;
	menu.button_pad = 2.0f;
	menu.center = true;
	menu.linesep = 2.4;

	menu_str("paused");

	menu_newline();

	settings_menu(cli->opts);

	menu_align((6 + 2 * menu.button_pad) * 2 + 1);
	menu.center = false;

	if (menu_button("resume", 0)) {
		cli->state &= ~csf_paused;
	}

	menu.x += 1;

	if (menu_button(" quit ", 0)) {
		cli->run = false;
	}
}

static void
render_element_buttons(struct gl_ui_ctx *ctx, struct client *cli)
{
	const float button_dim = 3, button_margin = 1.1f;
	const uint16_t button_count = ent_type_count + act_terrain_arg_count;

	menu.button_pad = button_dim;
	menu.x = (menu.gl_win->sc_width / menu.scale - (button_dim * button_margin * button_count)) / 2;
	menu.y = menu.gl_win->sc_height / menu.scale - button_dim;

	uint16_t t;
	for (t = 0; t < ent_type_count; ++t) {
		bool selected = cli->action == act_create && cli->action_arg == t;

		float sx = menu.x, sy = menu.y;
		if (menu_button_c(&(struct menu_button_ctx) {
			.w = button_dim,
			.clr = selected ? menu_theme_elem_bar_accent : 0
		})) {
			cli->action = act_create;
			cli->action_arg = t;
		}

		vec4 color;
		memcpy(color, colors.ent[t], sizeof(vec4));
		color[3] = 1.0f;
		render_shapes_add_rect(sx + button_dim / 4, sy + button_dim / 4,
			button_dim / 2, button_dim / 2, color);

		menu.x += button_dim * button_margin;
	}

	const char terrain_mod_syms[] = { 1, 0, 2, 0, '=', 0, };
	for (t = 0; t < act_terrain_arg_count; ++t) {
		bool selected = cli->action == act_terrain && cli->action_arg == t;

		if (menu_button_c(&(struct menu_button_ctx) {
			.str = &terrain_mod_syms[t * 2],
			.w = button_dim,
			.clr = selected ? menu_theme_elem_bar_accent : 0
		})) {
			cli->action = act_terrain;
			cli->action_arg = t;
		}
	}
}

void
render_hud(struct gl_ui_ctx *ctx, struct client *cli)
{
	TracyCZoneAutoS;

	const float mousex = ctx->sc_cursor.x * ctx->win->sc_width,
		    mousey = ctx->sc_cursor.y * ctx->win->sc_height;

	menu_begin(ctx->win, mousex, mousey, ctx->win->mouse.buttons & mb_1);

	menu.center = false;
	menu.linesep = 1.0;

	if (cli->im == im_cmd) {
		render_cmdline(ctx, cli);
	}

	if (ctx->debug_hud) {
		render_debug_hud(ctx, cli);
	}

	if (cli->state & csf_paused) {
		render_pause_menu(ctx, cli);
	}

	render_element_buttons(ctx, cli);

	menu_cursor(&(struct pointf){ mousex / menu.scale, mousey / menu.scale },
		menu_theme_elem_bar_accent);

	menu_render(ctx->win);

	ctx->cursor_on_world = !menu.something_was_hovered;

	TracyCZoneAutoE;
}
