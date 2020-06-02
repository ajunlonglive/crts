#include "posix.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "server/opts.h"
#include "shared/math/perlin.h"
#include "shared/math/rand.h"
#include "shared/util/log.h"

struct server_opts defaults = { 0 };

static void
set_default_opts(struct server_opts *so)
{
	*so = defaults;
}

static void
set_rand_seed(struct server_opts *so)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	rand_set_seed(ts.tv_nsec);
	so->seed = rand_uniform(0xffff);
}

static void
set_log_lvl(const char *otparg)
{
	log_level = strtol(optarg, NULL, 10);
}

static void
print_usage(void)
{
	printf("usage: crtsd [OPTIONS]\n"
		"\n"
		"OPTIONS:\n"
		"-s <seed>              - set seed\n"
		"-v <lvl>               - set verbosity\n"
		"-h                     - show this message\n"
		);
}

void
process_opts(int argc, char * const *argv, struct server_opts *so)
{
	signed char opt;
	bool seeded = false;

	set_default_opts(so);

	while ((opt = getopt(argc, argv, "hs:v:")) != -1) {
		switch (opt) {
		case 's':
			rand_set_seed(strtoul(optarg, NULL, 10));
			seeded = true;
			break;
		case 'v':
			set_log_lvl(optarg);
			break;
		case 'h':
			print_usage();
			exit(EXIT_SUCCESS);
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
			break;
		}
	}

	if (!seeded) {
		set_rand_seed(so);
	}

	perlin_noise_shuf();
}
