#ifndef LAUNCHER_OPTS_H
#define LAUNCHER_OPTS_H
#include "launcher/launcher.h"

bool parse_opts(int argc, char *const argv[], struct opts *opts);
bool parse_ip_address_opt(const char *addr, const char **ip, uint16_t *port, char **err_msg);
#endif
