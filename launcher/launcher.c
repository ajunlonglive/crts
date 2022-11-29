#include "posix.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "launcher/assets.h"
#include "launcher/launcher.h"
#include "launcher/opts.h"
#include "launcher/save_settings.h"
#include "shared/msgr/transport/basic.h"
#include "shared/msgr/transport/rudp.h"
#include "shared/platform/common/sockets.h"
#include "shared/platform/common/thread.h"
#include "shared/util/log.h"
#include "shared/util/timer.h"
#include "tracy.h"

#ifdef OPENGL_UI
#include "launcher/ui.h"
#endif

#ifdef CRTS_HAVE_client
#include "client/client.h"
#endif

#ifdef CRTS_HAVE_server
#include "server/server.h"
#endif

#ifdef CRTS_HAVE_terragen
#include "terragen/terragen.h"
#endif

static void
main_loop_client(struct runtime *rt)
{
	struct timer timer;
	timer_init(&timer);

	float client_tick_time = 0;

	if (rt->server_addr) {
		rudp_connect(rt->client->msgr, rt->server_addr);
	}

	rt->exit_reason = exit_reason_quit;

	while (*rt->run) {
		TracyCFrameMark;
		client_tick(rt->client);

		if (rt->server_addr) {
			if (!rudp_connected(rt->client->msgr, rt->server_addr)) {
				rt->exit_reason = exit_reason_disconnected;
				break;
			}
		}

		client_tick_time = timer_lap(&timer);

		timer_avg_push(&rt->client->prof.client_tick, client_tick_time);
	}
}

static void
main_loop_offline(struct runtime *rt)
{
	server_start(rt->server);

	main_loop_client(rt);

	server_stop();
}

static void
main_loop_server(struct runtime *rt)
{
	server_loop(rt->server);
}

static bool
setup_server(struct runtime *rt, struct server *server, struct opts *opts,
	const struct sock_impl *socks)
{
	static bool server_init = false;
	static struct msgr_transport_rudp_ctx server_rudp_ctx = { 0 };

	rt->server = server;

	if (!server_init) {
		if (!init_server(rt->server, &opts->launcher.wl)) {
			return false;
		}
		server_init = true;
	} else if (!reset_server(rt->server, &opts->launcher.wl)) {
		return false;
	}

	if (opts->launcher.mode & mode_online) {
		struct sock_addr addr;
		socks->addr_init(&addr, opts->launcher.net_addr.port);

		if (!msgr_transport_init_rudp(&server_rudp_ctx, &rt->server->msgr, socks, &addr)) {
			return false;
		}
	}

	return true;
}

static bool
setup_client(struct runtime *rt, struct client *client, struct opts *opts, const struct sock_impl *socks)
{
	static bool client_init = false;
	static struct msgr_transport_rudp_ctx client_rudp_ctx = { 0 };
	static struct sock_addr server_addr = { 0 };

	rt->client = client;

	if (!client_init) {
		if (!init_client(rt->client, &opts->client)) {
			return 1;
		}
		client_init = true;
	} else if (!reset_client(rt->client, &opts->client)) {
		return 1;
	}

	if (opts->launcher.mode & mode_online) {
		rt->server_addr = &server_addr;

		socks->addr_init(rt->server_addr, opts->launcher.net_addr.port);
		if (!socks->resolve(rt->server_addr, opts->launcher.net_addr.ip)) {
			return 1;
		}

		struct sock_addr addr = { 0 };
		if (!msgr_transport_init_rudp(&client_rudp_ctx, rt->client->msgr, socks, &addr)) {
			return 1;
		}
	} else {
		rt->server_addr = NULL;
	}

	rt->run = &rt->client->run;

	return true;
}

int
main(int argc, char *const argv[])
{
	log_init();
	launcher_assets_init();

	static struct server server = { 0 };
	static struct client client = { 0 };

	static bool always_true = true;
	struct runtime rt = { .run = &always_true };
	struct opts opts = {
		.client = {
			.sound = {
				.music = 0.0,
				.sfx = 60.0,
				.master = 90.0,
			},
			.ui_cfg = {
				.ui_scale = 20.0f,
				.mouse_sensitivity = 50.0f,
			},
		},
	};

	if (!parse_opts(argc, argv, &opts)) {
		return 1;
	}

	load_settings(&opts.client);

	const struct sock_impl *socks = NULL;

#ifdef OPENGL_UI
	struct launcher_ui_ctx launcher_ui_ctx;
#endif

	const enum mode original_mode = opts.launcher.mode;

	while (true) {
#ifdef OPENGL_UI
		if (opts.launcher.mode & mode_client && !opts.launcher.skip_menu) {
			launcher_ui_init(&launcher_ui_ctx, &opts);
			launcher_ui_ctx.exit_reason = rt.exit_reason;
			while (launcher_ui_ctx.run) {
				launcher_ui_render(&launcher_ui_ctx);
			}

			if (launcher_ui_ctx.exit) {
				break;
			}
		}
#endif

		if (opts.launcher.mode & mode_online && !socks) {
			socks = get_sock_impl(sock_impl_type_system);
			socks->init();
		}

		if ((opts.launcher.mode & mode_server)
		    && !setup_server(&rt, &server, &opts, socks)) {
			return 1;
		}

		if ((opts.launcher.mode & mode_client)
		    && !setup_client(&rt, &client, &opts, socks)) {
			return 1;
		}

		if (!(opts.launcher.mode & mode_online)) {
			msgr_transport_init_basic_pipe(rt.client->msgr, &rt.server->msgr);
			main_loop_offline(&rt);
		} else if (opts.launcher.mode & mode_server) {
			main_loop_server(&rt);
		} else {
			main_loop_client(&rt);
		}

#ifndef OPENGL_UI
		break;
#endif
		if (opts.launcher.skip_menu) {
			break;
		}

		opts.launcher.mode = original_mode;
	}

	if (rt.client) {
		deinit_client(rt.client);
	}

	if (socks) {
		socks->deinit();
	}

	save_settings(&opts.client);
}
