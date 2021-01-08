#ifndef SERVER_OPTS_H
#define SERVER_OPTS_H

#include "shared/sim/world.h"

struct server_opts {
	int dummy;
};

void parse_server_opts(int argc, char * const *argv, struct server_opts *so);
#endif
