#ifndef SERVER_OPTS_H
#define SERVER_OPTS_H

#include "terragen/gen/gen.h"

struct server_opts {
	long seed;
	terragen_opts tg_opts;
};

void process_s_opts(int argc, char * const *argv, struct server_opts *so);
#endif
