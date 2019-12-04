#define _POSIX_C_SOURCE 199309L

#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "util/log.h"
#include "traps.h"

static void handle_sigint(_)
{
	L("shutting down");

	exit(0);
}

void trap_sigint()
{
	struct sigaction sigact;

	memset(&sigact, 0, sizeof(struct sigaction));

	sigact.sa_flags = 0;
	sigact.sa_handler = handle_sigint;
	sigaction(SIGINT, &sigact, NULL);
}
