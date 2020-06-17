#ifndef CLIENT_OPTS_H
#define CLIENT_OPTS_H

#include <stdint.h>

#define OPT_STR_VALUE_LEN 127

struct c_opts {
	char logfile[OPT_STR_VALUE_LEN + 1];
	char ip_addr[OPT_STR_VALUE_LEN + 1];
	char asset_path[OPT_STR_VALUE_LEN + 1];
	uint8_t ui;
	long id;
};

void process_c_opts(int argc, char * const *argv, struct c_opts *opts);
#endif
