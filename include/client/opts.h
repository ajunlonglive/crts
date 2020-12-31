#ifndef CLIENT_OPTS_H
#define CLIENT_OPTS_H

#include <stdint.h>

enum client_mode {
	client_mode_offline,
	client_mode_online,
	client_mode_map_viewer,
};

struct client_opts {
	uint8_t ui;
	uint16_t id;
	enum client_mode mode;
	const char *ip_addr;
	const char *load_map;
	const char *cmds;
};

void process_client_opts(int argc, char * const *argv, struct client_opts *opts);
#endif
