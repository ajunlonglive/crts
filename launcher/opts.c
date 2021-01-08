#include "posix.h"

#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "client/opts.h"
#include "launcher/opts.h"
#include "server/opts.h"
#include "shared/constants/port.h"
#include "shared/math/rand.h"
#include "shared/serialize/to_disk.h"
#include "shared/util/log.h"
#include "terragen/gen/gen.h"
#include "terragen/gen/opts.h"
#include "version.h"

static char *default_ip = "127.0.0.1";

enum feature {
	feat_server,
	feat_client,
	feat_terragen,
	feature_count,
};

static const bool have_feature[feature_count] = {
#ifdef CRTS_HAVE_server
	[feat_server] = true,
#endif
#ifdef CRTS_HAVE_client
	[feat_client] = true,
#endif
#ifdef CRTS_HAVE_terragen
	[feat_terragen] = true,
#endif
};

static const char *feature_name[feature_count] = {
	[feat_server] = "server",
	[feat_client] = "client",
	[feat_terragen] = "terragen",
};

static void
print_usage(const char *argv0)
{
	printf("crts v%s-%s\n"
		"usage: %s [opts] [subcmd [opts] [subcmd [opts] [...]]]\n"
		"\n"
		"OPTIONS:\n"
		"  -a <path[:path[:path]]> - set asset path\n"
		"  -v <lvl>                - set verbosity\n"
		"  -l <file>               - log to <file>\n"
		"  -w <file>               - load world from <file>\n"
		"  -g <opts>               - generate world from <opts>\n"
		"  -s <seed>               - set seed\n"
		"  -n <host:port>          - set network address\n"
		"  -h                      - show this message\n"
		"compiled features:\n",
		crts_version.version,
		crts_version.vcs_tag,
		argv0);

	uint32_t i;
	for (i = 0; i < feature_count; ++i) {
		if (have_feature[i]) {
			printf("  %s\n", feature_name[i]);
		}
	}
}

static bool
world_loader_terragen(struct world *w, char *opts)
{
	terragen_opts tg_opts = { 0 };
	tg_opts_set_defaults(tg_opts);
	tg_parse_optstring(opts, tg_opts);

	LOG_I("generating world");

	struct terragen_ctx ctx = { 0 };
	terragen_init(&ctx, tg_opts);
	terragen(&ctx, &w->chunks);
	return true;
}

static bool
world_loader_from_file(struct world *w, char *path)
{
	return load_world_from_path(path, &w->chunks);
}

static bool
parse_launcher_opts(int argc, char *const argv[], struct launcher_opts *opts)
{
	signed char opt;

	bool seeded = false;
	char *p;

	while ((opt = getopt(argc, argv,  "n:g:w:l:v:s:a:")) != -1) {
		switch (opt) {
		case 'v':
			set_log_lvl(optarg);
			break;
		case 'a':
			asset_path_init(optarg);
			break;
		case 'l':
			set_log_file(optarg);
			break;
		case 's':
			rand_set_seed(strtoul(optarg, NULL, 10));
			seeded = true;
			break;
		case 'w':
			opts->wl.loader = world_loader_from_file;
			opts->wl.opts = optarg;
			break;
		case 'g':
			opts->wl.loader = world_loader_terragen;
			opts->wl.opts = optarg;
			break;
		case 'n':
			opts->net_addr.ip = default_ip;
			opts->net_addr.port = PORT;

			if ((p = strchr(optarg, ':'))) {
				*p = 0;

				if (*(p + 1)) {
					opts->net_addr.port = strtol(p + 1, NULL, 10);
				}

				if (p != optarg) {
					opts->net_addr.ip = optarg;
				}
			} else {
				opts->net_addr.ip = optarg;
			}

			opts->mode |= mode_online;
			break;
		default:
			return false;
		}
	}

	if (!seeded) {
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		rand_set_seed(ts.tv_nsec);
	}

	return true;
}

bool
get_subcmd(const char *cmd, enum feature *feat)
{
	uint32_t i;
	for (i = 0; i < feature_count; ++i) {
		if (strcmp(feature_name[i], cmd) == 0) {
			if (have_feature[i]) {
				*feat = i;
				return true;
			} else {
				LOG_W("requested feature '%s' is not available", cmd);
				return false;
			}

		}
	}

	LOG_W("unknown feature '%s'", cmd);
	return false;
}

bool
parse_opts(int argc, char *const argv[], struct opts *opts)
{
	if (!parse_launcher_opts(argc, argv, &opts->launcher)) {
		goto usage_err;
	}

	enum feature feat;
	while (optind < argc) {
		if (!get_subcmd(argv[optind], &feat)) {
			goto usage_err;
		}
		++optind;

		switch (feat) {
		case feat_client:
			parse_client_opts(argc, argv, &opts->client);
			opts->launcher.mode |= mode_client;
			break;
		case feat_server:
			parse_server_opts(argc, argv, &opts->server);
			opts->launcher.mode |= mode_server;
			break;
		default:
			LOG_W("not yet implemented");
			return false;
		}
	}

	if (opts->launcher.mode & mode_online) {
		if (!(opts->launcher.mode & (mode_client | mode_server))) {
			opts->launcher.mode |= mode_client;
		}
	} else {
		opts->launcher.mode |= (mode_client | mode_server);
	}

	return true;
usage_err:
	print_usage(argv[0]);
	return false;
}
