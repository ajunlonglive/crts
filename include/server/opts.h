#ifndef SERVER_OPTS_H
#define SERVER_OPTS_H

struct server_opts {
	long seed;
};

void process_s_opts(int argc, char * const *argv, struct server_opts *so);
#endif
