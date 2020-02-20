#define _XOPEN_SOURCE 500

#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "server/traps.h"
#include "shared/util/log.h"

static void
handle_sigint(int _)
{
	L("shutting down");

	exit(0);
}

void
trap_sigint(void)
{
	struct sigaction sigact;

	memset(&sigact, 0, sizeof(struct sigaction));

	sigact.sa_flags = 0;
	sigact.sa_handler = handle_sigint;
	sigaction(SIGINT, &sigact, NULL);
}
