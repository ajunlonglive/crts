#include "posix.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared/serialize/to_disk.h"
#include "terragen/gen/gen.h"
#include "terragen/terragen.h"
#include "shared/util/log.h"

#ifdef OPENGL_UI
#include "terragen/opengl/ui.h"
#endif

static void
print_usage(void)
{
	printf("usage: terragen [opts]\n"
		"\n"
		"opts:\n"
		"-d                      - disable the ui, run headless\n"
		"-w <file>               - set the output file\n"
		"-o <opts>               - set world generation options\n"
		"-h                      - show this message\n"
		);
}


void
parse_terragen_opts(int argc, char * const *argv, struct terragen_opts *opts)
{
	signed char opt;

	tg_opts_set_defaults(opts->opts);

#ifdef OPENGL_UI
	opts->interactive = true;
#endif
	opts->world_file = "world.crw";

	while ((opt = getopt(argc, argv, "w:do:h")) != -1) {
		switch (opt) {
		case 'w':
			opts->world_file = optarg;
			break;
		case 'd':
			opts->interactive = false;
			break;
		case 'o':
			tg_parse_optstring(optarg, opts->opts);
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
}

void
terragen_main(struct terragen_opts *opts)
{
	if (opts->interactive) {
#ifdef OPENGL_UI
		genworld_interactive(opts->opts, opts->world_file);
#else
		LOG_W("built without opengl, interactive mode unsupported");
#endif
	} else {
		struct chunks chunks;
		chunks_init(&chunks);

		struct terragen_ctx ctx = { 0 };

		terragen_init(&ctx, opts->opts);

		LOG_I("generating world");
		terragen(&ctx, &chunks);

		LOG_I("saving to '%s'", opts->world_file);

		FILE *f;
		if (strcmp(opts->world_file, "-") == 0) {
			f = stdout;
		} else if (!(f = fopen(opts->world_file, "w"))) {
			LOG_W("unable write to file '%s'", opts->world_file);
			return;
		}

		write_chunks(f, &chunks);
		fclose(f);
	}
}
