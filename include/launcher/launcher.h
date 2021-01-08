#ifndef LAUNCHER_LAUNCHER_H
#define LAUNCHER_LAUNCHER_H

#include "client/client.h"
#include "server/server.h"
#include "shared/platform/sockets/common.h"

enum mode {
	mode_server   = 1 << 0,
	mode_client   = 1 << 1,
	mode_online   = 1 << 2,
	mode_terragen = 1 << 3,
};

struct runtime {
	struct server *server;
	struct client *client;
	struct sock_addr *server_addr;
	void ((*tick)(struct runtime*));
	bool *run;
};

struct launcher_opts {
	struct world_loader wl;
	struct {
		const char *ip;
		uint16_t port;
	} net_addr;
	enum mode mode;
};

struct opts {
	struct launcher_opts launcher;
	struct client_opts client;
	struct server_opts server;
};
#endif
