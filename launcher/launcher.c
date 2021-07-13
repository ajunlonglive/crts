#include "posix.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "launcher/assets.h"
#include "launcher/launcher.h"
#include "launcher/opts.h"
#include "shared/msgr/transport/basic.h"
#include "shared/msgr/transport/rudp.h"
#include "shared/platform/common/sockets.h"
#include "shared/platform/common/thread.h"
#include "shared/util/log.h"
#include "shared/util/timer.h"
#include "tracy.h"

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
main_loop(struct runtime *rt)
{
	struct timer timer;
	timer_init(&timer);

	float client_tick_time = 0;

	if (rt->server) {
		server_start(rt->server);
	}

	while (*rt->run) {
		TracyCFrameMark;
		client_tick(rt->client);

		if (rt->server_addr) {
			rudp_connect(rt->client->msgr, rt->server_addr);
		}

		client_tick_time = timer_lap(&timer);

		timer_avg_push(&rt->client->prof.client_tick, client_tick_time);
	}

	server_stop();
}

int
main(int argc, char *const argv[])
{
	log_init();
	launcher_assets_init();

	static struct server server = { 0 };
	static struct client client = { 0 };
	static struct sock_addr server_addr = { 0 };
	static struct msgr_transport_rudp_ctx server_rudp_ctx = { 0 }, client_rudp_ctx = { 0 };

	static bool always_true = true;
	struct runtime rt = { .run = &always_true };
	struct opts opts = { 0 };

	if (!parse_opts(argc, argv, &opts)) {
		return 1;
	}

	const struct sock_impl *socks = NULL;
	bool client_init = false, server_init = false;

	while (true) {

		if (opts.launcher.mode & mode_online && !socks) {
			socks = get_sock_impl(sock_impl_type_system);
			socks->init();
		}

#ifdef CRTS_HAVE_server
		if (opts.launcher.mode & mode_server) {
			rt.server = &server;

			if (!server_init) {
				if (!init_server(rt.server, &opts.launcher.wl)) {
					return 1;
				}
				server_init = true;
			} else if (!reset_server(rt.server, &opts.launcher.wl)) {
				return 1;
			}

			if (opts.launcher.mode & mode_online) {
				struct sock_addr addr;
				socks->addr_init(&addr, opts.launcher.net_addr.port);

				if (!msgr_transport_init_rudp(&server_rudp_ctx, &rt.server->msgr, socks, &addr)) {
					return 1;
				}
			}
		}
#endif

#ifdef CRTS_HAVE_client
		if (opts.launcher.mode & mode_client) {
			rt.client = &client;

			if (!client_init) {
				if (!init_client(rt.client, &opts.client)) {
					return 1;
				}
				client_init = true;
			} else if (!reset_client(rt.client, &opts.client)) {
				return 1;
			}

			if (opts.launcher.mode & mode_online) {
				rt.server_addr = &server_addr;

				socks->addr_init(rt.server_addr, opts.launcher.net_addr.port);
				if (!socks->resolve(rt.server_addr, opts.launcher.net_addr.ip)) {
					return 1;
				}

				struct sock_addr addr = { 0 };
				if (!msgr_transport_init_rudp(&client_rudp_ctx, rt.client->msgr, socks, &addr)) {
					return 1;
				}
			}

			rt.run = &rt.client->run;
		}
#endif

		if (!(opts.launcher.mode & mode_online)) {
			msgr_transport_init_basic_pipe(rt.client->msgr, &rt.server->msgr);
		}

		main_loop(&rt);
	}
/* #ifdef CRTS_HAVE_client */
/* 	if (opts.launcher.mode & mode_client) { */
/* 		deinit_client(rt.client); */
/* 	} */
/* #endif */

/* #if defined(CRTS_HAVE_client) || defined(CRTS_HAVE_server) */
/* 	if (socks) { */
/* 		socks->deinit(); */
/* 	} */
/* #endif */
}
