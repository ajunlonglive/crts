#include "posix.h"

#include "client/client.h"
#include "client/opts.h"
#include "shared/util/log.h"
#include "shared/util/time.h"

#define TICK NS_IN_S / 30

int
main(int argc, char * const *argv)
{
	log_init();
	struct timespec tick_st;
	struct client_opts opts = { 0 };
	process_client_opts(argc, argv, &opts);

	struct client cli = { 0 };

	if (!init_client(&cli, &opts)) {
		LOG_W("failed to initialize client");
		return 1;
	}

	long slept_ns = 0;
	clock_gettime(CLOCK_MONOTONIC, &tick_st);

	while (cli.run) {
		cli.tick(&cli);

		slept_ns = sleep_remaining(&tick_st, TICK, slept_ns);
	}

	L("shutting down");
	deinit_client(&cli);
}
