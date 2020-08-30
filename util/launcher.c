#include "posix.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef LIBEXECDIR
#define LIBEXECDIR ""
#endif

static const struct {
	const char *name, alias, *desc;
} subcmds[] = {
	"client", 'c', "crts client",
	"server", 's', "crts server",
	"terragen", 't', "interactively generate worlds",
	"snap", 'p', "create an image from a save file",
	0
};

static int32_t
detect_subcmd(const char *cmd)
{
	uint32_t i;
	size_t len = strlen(cmd);

	for (i = 0; subcmds[i].name; ++i) {
		if ((len == 1 && *cmd == subcmds[i].alias)
		    || strncmp(cmd, subcmds[i].name, len) == 0) {
			return i;
		}
	}

	return -1;
}

static void
print_usage(void)
{
	uint32_t i;

	printf("usage: crts SUBCMD\navailable subcmds:\n");

	for (i = 0; subcmds[i].name; ++i) {
		printf("  %c, %-10.10s - %s\n", subcmds[i].alias,
			subcmds[i].name, subcmds[i].desc);
	}
}

int32_t
main(int32_t argc, char *const argv[])
{
	int32_t cmdi;

	if (argc < 2) {
		fprintf(stderr, "missing subcmd\n");
		print_usage();
		return 1;
	} else if ((cmdi = detect_subcmd(argv[1])) == -1) {
		fprintf(stderr, "invalid subcmd: %s\n", argv[1]);
		print_usage();
		return 1;
	}

	size_t blen = strlen(LIBEXECDIR) + strlen(subcmds[cmdi].name) + 2;
	char path[blen];
	memset(path, 0, blen);

	snprintf(path, blen, "%s/%s", LIBEXECDIR, subcmds[cmdi].name);

	if (execv(path, &argv[1]) == -1) {
		printf("error executing subcommand: '%s'\n  path: %s\n  error: %s\n",
			subcmds[cmdi].name,
			path,
			strerror(errno));
	}
}
