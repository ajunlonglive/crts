#ifndef CLIENT_OPTS_H
#define CLIENT_OPTS_H

#include <stdbool.h>
#include <stdint.h>

struct client_opts {
	uint8_t ui;
	uint16_t id;
	bool mute;
	const char *cmds;
};

void parse_client_opts(int argc, char * const *argv, struct client_opts *opts);
#endif
