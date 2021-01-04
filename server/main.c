#include "posix.h"

#include "server/api.h"
#include "shared/constants/port.h"
#include "shared/msgr/transport/rudp.h"
#include "shared/util/log.h"
#include "shared/util/time.h"
#include "tracy.h"

#define TICK NS_IN_S / 30

int
main(int argc, char * const *argv)
{
	log_init();
	struct timespec tick_st;
	struct server_opts opts = { 0 };
	process_server_opts(argc, argv, &opts);

	struct server server = { 0 };

	if (!init_server(&server, &opts)) {
		LOG_W("failed to initialize server");
		return 1;
	}

	const struct sock_impl *impl = get_sock_impl(sock_impl_type_system);
	struct sock_addr addr;
	impl->addr_init(&addr, PORT);

	if (!msgr_transport_init_rudp(&server.msgr, impl, &addr)) {
		LOG_W("failed to initialize msgr transport");
		return 1;
	}

	long slept_ns = 0;
	clock_gettime(CLOCK_MONOTONIC, &tick_st);

	while (1) {
		TracyCFrameMark;

		server_tick(&server);

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);
	}
}
