#include "posix.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "client/opts.h"
#include "client/ui/common.h"
#include "shared/math/rand.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"
#include "version.h"

struct c_opts defaults = {
	.ip_addr = "127.0.0.1",
	.ui = ui_default,
};

struct cfg_lookup_table uis = {
#ifdef NCURSES_UI
	"ncurses", ui_ncurses,
#endif
#ifdef OPENGL_UI
	"opengl",  ui_opengl,
#ifdef NCURSES_UI
	"both", ui_ncurses | ui_opengl,
#endif
#endif
	"null", ui_null,
};

static void
set_default_opts(struct c_opts *opts)
{
	*opts = defaults;
}

static void
set_rand_id(struct c_opts *opts)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	rand_set_seed(ts.tv_nsec);
	opts->id = rand_uniform(0xffff);
}

static void
print_usage(void)
{
	printf("crts v%s-%s\n"
		"usage: crts [OPTIONS]\n"
		"\n"
		"OPTIONS:\n"
		"-a <path[:path[:path]]> - set asset path\n"
		"-i <integer>            - set client id\n"
		"-s <ip address>         - set server ip\n"
		"-o <UI>                 - enable UI\n"
		"-v <lvl>                - set verbosity\n"
		"-l <file>               - log to <file>\n"
		"-h                      - show this message\n"
		"\n"
		"Available UIs: "
#ifdef NCURSES_UI
		"ncurses, "
#endif
#ifdef OPENGL_UI
		"opengl, "
#ifdef NCURSES_UI
		"both, "
#endif
#endif
		"null\n",
		VERSION,
		VCS_TAG
		);
}

static uint32_t
parse_ui_str(const char *str, uint32_t cur)
{
	int32_t bit;

	if ((bit = cfg_string_lookup(str, &uis)) < 0) {
		fprintf(stderr, "invalid ui: %s\n", str);
		exit(EXIT_FAILURE);
	}

	if (cur == ui_default) {
		cur = 0;
	}

	return cur | bit;
}

void
process_c_opts(int argc, char * const *argv, struct c_opts *opts)
{
	signed char opt;
	bool id_set = false;

	set_default_opts(opts);

	while ((opt = getopt(argc, argv,  "a:hi:o:s:l:v:")) != -1) {
		switch (opt) {
		case 'a':
			asset_path_init(optarg);
			break;
		case 'h':
			print_usage();
			exit(EXIT_SUCCESS);
			break;
		case 'i':
			opts->id = strtol(optarg, NULL, 10);
			id_set = true;
			break;
		case 'o':
			opts->ui = parse_ui_str(optarg, opts->ui);
			break;
		case 's':
			strncpy(opts->ip_addr, optarg, OPT_STR_VALUE_LEN);
			break;
		case 'l':
			set_log_file(optarg);
			break;
		case 'v':
			set_log_lvl(optarg);
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
			break;
		}
	}

	if (!id_set) {
		set_rand_id(opts);
	}

	if (opts->ui == ui_default) {
#ifdef OPENGL_UI
		L("using default ui: opengl");
		opts->ui = ui_opengl;
#else
#ifdef NCURSES_UI
		L("using default ui: ncurses");
		opts->ui = ui_ncurses;
#else
		L("using default ui: null");
		opts->ui = ui_null;
#endif
#endif
	}

	L("client id: %ld", opts->id);
}
