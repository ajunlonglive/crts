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
#include "shared/platform/sockets/common.h"
#include "shared/util/log.h"
#include "shared/util/time.h"

#ifdef CRTS_HAVE_client
#include "client/client.h"
#endif

#ifdef CRTS_HAVE_server
#include "server/server.h"
#endif

#define TICK NS_IN_S / 30

static void
main_loop(struct runtime *rt)
{
	struct timespec tick_st;

	long slept_ns = 0;
	clock_gettime(CLOCK_MONOTONIC, &tick_st);

	while (*rt->run) {
#ifdef CRTS_HAVE_server
		if (rt->server) {
			server_tick(rt->server);
		}
#endif

#ifdef CRTS_HAVE_client
		if (rt->client) {
			client_tick(rt->client);

			if (rt->server_addr) {
				rudp_connect(rt->client->msgr, rt->server_addr);
			}
		}
#endif

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);
	}
}

int
main(int argc, char *const argv[])
{
	log_init();
	launcher_assets_init();

	static bool always_true = true;
	struct runtime rt = { .run = &always_true };
	struct opts opts = { 0 };

	const struct sock_impl *socks = NULL;

	if (!parse_opts(argc, argv, &opts)) {
		return 1;
	}

	if (opts.launcher.mode & mode_online) {
		socks = get_sock_impl(sock_impl_type_system);
	}

#ifdef CRTS_HAVE_server
	if (opts.launcher.mode & mode_server) {
		static struct server server = { 0 };
		rt.server = &server;

		if (!init_server(rt.server, &opts.launcher.wl, &opts.server)) {
			return 1;
		}

		if (opts.launcher.mode & mode_online) {
			struct sock_addr addr;
			socks->addr_init(&addr, opts.launcher.net_addr.port);

			static struct msgr_transport_rudp_ctx server_rudp_ctx = { 0 };
			if (!msgr_transport_init_rudp(&server_rudp_ctx, &rt.server->msgr, socks, &addr)) {
				return 1;
			}
		}
	}
#endif

#ifdef CRTS_HAVE_client
	if (opts.launcher.mode & mode_client) {
		static struct client client = { 0 };
		rt.client = &client;

		if (!init_client(rt.client, &opts.client)) {
			return 1;
		}

		if (opts.launcher.mode & mode_online) {
			static struct sock_addr server_addr = { 0 };
			rt.server_addr = &server_addr;

			socks->addr_init(rt.server_addr, opts.launcher.net_addr.port);
			if (!socks->resolve(rt.server_addr, opts.launcher.net_addr.ip)) {
				return 1;
			}

			struct sock_addr addr = { 0 };
			static struct msgr_transport_rudp_ctx client_rudp_ctx = { 0 };
			if (!msgr_transport_init_rudp(&client_rudp_ctx, rt.client->msgr, socks, &addr)) {
				return 1;
			}
		}

		rt.run = &rt.client->run;
	}
#endif

#if defined(CRTS_HAVE_client) && defined(CRTS_HAVE_server)
	if (!(opts.launcher.mode & mode_online)) {
		static struct msgr_transport_basic_ctx transport_basic_ctx[2] = { 0 };

		msgr_transport_init_basic(rt.client->msgr, &rt.server->msgr,
			&transport_basic_ctx[0]);
		msgr_transport_init_basic(&rt.server->msgr, rt.client->msgr,
			&transport_basic_ctx[1]);
	}
#endif

	main_loop(&rt);

#ifdef CRTS_HAVE_client
	if (opts.launcher.mode & mode_client) {
		deinit_client(rt.client);
	}
#endif
}
