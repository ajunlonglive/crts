#include "posix.h"

#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "shared/serialize/to_disk.h"
#include "shared/util/assets.h"
#include "shared/util/log.h"
#include "terragen/gen/gen.h"
#include "terragen/gen/opts.h"

#ifdef OPENGL_UI
#include "terragen/opengl/ui.h"
#endif

static void
print_usage(void)
{
	printf("terragen - generate terrain for crts\n"
		"usage: genworld [OPTIONS] [outfile]\n"
		"\n"
		"OPTIONS:\n"
		"-a <path[:path[:path]]> - set asset path\n"
		"-o opt1=val1[,opt2=val2[,...]]\n"
		"                        - set world generation options\n"
		"-i FILE                 - read world generation options from file\n"
		"-v <lvl>                - set verbosity\n"
		"-l <file>               - log to <file>\n"
		"-h                      - show this message\n"
		);
}

struct cmdline_opts {
	terragen_opts opts;
	bool interactive;
	char *outfile;
};

void
parse_cmdline_opts(int32_t argc, char *const *argv, struct cmdline_opts *opts)
{
	signed char opt;

	while ((opt = getopt(argc, argv, "a:f:hil:o:v:")) != -1) {
		switch (opt) {
		case 'a':
			asset_path_init(optarg);
			break;
		case 'f':
			tg_parse_optfile(rel_to_abs_path(optarg), opts->opts);
			break;
		case 'h':
			print_usage();
			exit(EXIT_SUCCESS);
			break;
		case 'i':
			opts->interactive = true;
			break;
		case 'l':
			set_log_file(optarg);
			break;
		case 'o':
			tg_parse_optstring(optarg, opts->opts);
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

	if (optind == argc) {
		opts->outfile = "world.crw";
	} else {
		opts->outfile = argv[optind];
	}
}

int32_t
main(int32_t argc, char * const *argv)
{
	logfile = stderr;

	setlocale(LC_ALL, "");
	struct cmdline_opts opts = { 0 };
	tg_opts_set_defaults(opts.opts);
	parse_cmdline_opts(argc, argv, &opts);

	if (opts.interactive) {
#ifdef OPENGL_UI
		genworld_interactive(opts.opts, opts.outfile);
		return 0;
#else
		LOG_W("built without opengl, interactive mode unsupported");
		return 1;
#endif
	} else {
		struct chunks chunks;
		chunks_init(&chunks);

		struct terragen_ctx ctx = { 0 };

		terragen_init(&ctx, opts.opts);

		terragen(&ctx, &chunks);

		FILE *f;
		if (strcmp(opts.outfile, "-") == 0) {
			f = stdout;
		} else if (!(f = fopen(opts.outfile, "w"))) {
			LOG_W("unable write to file '%s'", opts.outfile);
			return 1;
		}

		write_chunks(f, &chunks);
		fclose(f);
	}
}
