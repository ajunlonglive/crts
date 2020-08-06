#include "posix.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "server/opts.h"
#include "shared/math/perlin.h"
#include "shared/math/rand.h"
#include "shared/util/assets.h"
#include "shared/util/log.h"
#include "version.h"

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
print_usage(void)
{
	printf("crtsd v%s-%s\n"
		"usage: crtsd [OPTIONS]\n"
		"\n"
		"OPTIONS:\n"
		"-a <path[:path[:path]]> - set asset path\n"
		"-s <seed>               - set seed\n"
		"-v <lvl>                - set verbosity\n"
		"-h                      - show this message\n",
		VERSION,
		VCS_TAG
		);
}

void
process_s_opts(int argc, char *const *argv, struct server_opts *so)
{
	signed char opt;
	bool seeded = false;

	set_default_opts(so);

	while ((opt = getopt(argc, argv, "a:hs:v:")) != -1) {
		switch (opt) {
		case 'a':
			asset_path_init(optarg);
			break;
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
