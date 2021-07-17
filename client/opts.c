#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "client/opts.h"
#include "client/ui/common.h"
#include "shared/util/inih.h"
#include "shared/util/log.h"

static struct cfg_lookup_table uis = {
#ifdef NCURSES_UI
	"term", ui_term,
#endif
#ifdef OPENGL_UI
	"gl",  ui_gl,
#ifdef NCURSES_UI
	"both", ui_term | ui_gl,
#endif
#endif
	"null", ui_null,
};

static void
print_usage(void)
{
	printf("usage: client [opts]\n"
		"\n"
		"opts:\n"
		"-i <integer>            - set client id\n"
		"-o <UI>                 - enable UI\n"
		"-c <cmd[;cmd[;...]]>    - execude cmd(s) on startup\n"
		"-m                      - disable (mute) sound\n"
		"-h                      - show this message\n"
		"\n"
		"Available UIs: "
#ifdef NCURSES_UI
		"term, "
#endif
#ifdef OPENGL_UI
		"gl, "
#ifdef NCURSES_UI
		"both, "
#endif
#endif
		"null\n"
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
parse_client_opts(int argc, char * const *argv, struct client_opts *opts)
{
	signed char opt;

	while ((opt = getopt(argc, argv,  "c:i:o:mh")) != -1) {
		switch (opt) {
		case 'c':
			opts->cmds = optarg;
			break;
		case 'i':
			opts->id = strtol(optarg, NULL, 10);
			break;
		case 'o':
			opts->ui = parse_ui_str(optarg, opts->ui);
			break;
		case 'm':
			opts->sound.disable = true;
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
