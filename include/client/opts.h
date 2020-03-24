#ifndef CLIENT_OPTS_H
#define CLIENT_OPTS_H

#define OPT_STR_VALUE_LEN 128

struct opts {
	struct {
		char graphics[OPT_STR_VALUE_LEN];
		char keymap[OPT_STR_VALUE_LEN];
	} cfg;
	char ip_addr[OPT_STR_VALUE_LEN];
	long id;
};

void process_opts(int argc, char * const *argv, struct opts *opts);
#endif
