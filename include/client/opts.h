#ifndef CLIENT_OPTS_H
#define CLIENT_OPTS_H

#include <stdint.h>

struct c_opts {
	uint8_t ui;
	uint16_t id;
	const char *ip_addr;
	const char *load_map;
};

void process_c_opts(int argc, char * const *argv, struct c_opts *opts);
#endif
