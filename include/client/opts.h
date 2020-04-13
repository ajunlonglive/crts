#ifndef CLIENT_OPTS_H
#define CLIENT_OPTS_H

#include <stdint.h>

#define OPT_STR_VALUE_LEN 127

struct opts {
	struct {
		char graphics[OPT_STR_VALUE_LEN + 1];
		char keymap[OPT_STR_VALUE_LEN + 1];
	} cfg;
	char ip_addr[OPT_STR_VALUE_LEN + 1];
	uint8_t ui;
	long id;
};

void process_opts(int argc, char * const *argv, struct opts *opts);
#endif
