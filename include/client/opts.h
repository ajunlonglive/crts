#ifndef CLIENT_OPTS_H
#define CLIENT_OPTS_H
struct opts {
	long id;
	char ip_addr[32];
};

void process_opts(int argc, char * const *argv, struct opts *opts);
#endif
