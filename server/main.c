#include "posix.h"

#include "server/api.h"
#include "shared/util/log.h"
#include "shared/util/time.h"
#include "tracy.h"

#define TICK NS_IN_S / 30

int
main(int argc, char * const*argv)
{
	log_init();
	struct timespec tick_st;
	struct server_opts so = { 0 };
	process_s_opts(argc, argv, &so);

	struct server s = { 0 };

	if (!init_server(&s, &so)) {
		LOG_W("failed to initialize server");
		return 1;
	}

	long slept_ns = 0;
	clock_gettime(CLOCK_MONOTONIC, &tick_st);

	while (1) {
		TracyCFrameMark;

		server_tick(&s);

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);
	}
}
