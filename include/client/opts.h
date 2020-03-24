#ifndef CLIENT_OPTS_H
#define CLIENT_OPTS_H
struct opts {
	struct {
		char graphics[128];
		char keymap[128];
	} cfg;
	char ip_addr[32];
	long id;
};

void process_opts(int argc, char * const *argv, struct opts *opts);
#endif
