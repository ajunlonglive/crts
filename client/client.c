#include "posix.h"

#include <string.h>

#include "client/assets.h"
#include "client/cfg/keymap.h"
#include "client/client.h"
#include "client/handle_msg.h"
#include "client/input/helpers.h"
#include "client/opts.h"
#include "client/request_missing_chunks.h"
#include "client/ui/common.h"
#include "server/api.h"
#include "shared/constants/port.h"
#include "shared/msgr/transport/basic.h"
#include "shared/msgr/transport/rudp.h"
#include "shared/platform/sockets/common.h"
#include "shared/serialize/to_disk.h"
#include "shared/util/log.h"

static struct server server = { 0 };
static struct sock_addr server_addr = { 0 };

static void
tick_basic(struct client *cli)
{
	ui_handle_input(cli);
	ui_render(cli);
	memset(&cli->changed, 0, sizeof(cli->changed));
}

static void
tick_offline(struct client *cli)
{
	server_tick(&server);

	request_missing_chunks(cli);
	msgr_send(cli->msgr);
	msgr_recv(cli->msgr);

	tick_basic(cli);
}

static bool
init_offline(struct client *cli)
{
	struct server_opts opts = { .world = "w2.crw" };
	if (!init_server(&server, &opts)) {
		LOG_W("failed to initialize server");
		return false;
	}

	msgr_transport_init_basic(cli->msgr, &server.msgr);
	msgr_transport_init_basic(&server.msgr, cli->msgr);

	request_missing_chunks_init();
	cli->tick = tick_offline;

	return true;
}

static void
tick_online(struct client *cli)
{
	request_missing_chunks(cli);

	msgr_transport_connect(cli->msgr, &server_addr);

	msgr_send(cli->msgr);
	msgr_recv(cli->msgr);

	tick_basic(cli);
}

static bool
init_online(struct client *cli, struct client_opts *opts)
{
	const struct sock_impl *impl = get_sock_impl(sock_impl_type_system);

	impl->addr_init(&server_addr, PORT);
	if (!impl->resolve(&server_addr, opts->ip_addr)) {
		return false;
	}

	struct sock_addr client_addr = { 0 };

	if (!msgr_transport_init_rudp(cli->msgr, impl, &client_addr)) {
		return false;
	}

	request_missing_chunks_init();
	cli->tick = tick_online;

	return true;
}

void
deinit_client(struct client *cli)
{
	ui_deinit(cli->ui_ctx);
}

bool
init_client(struct client *cli, struct client_opts *opts)
{
	client_assets_init();

	cli->id = opts->id;
	cli->run = true;

	static struct msgr msgr = { 0 };
	msgr_init(&msgr, cli, client_handle_msg, cli->id);
	cli->msgr = &msgr;

	static struct world world = { 0 };
	world_init(&world);
	cli->world = &world;

	static struct ui_ctx ui_ctx = { 0 };
	ui_init(opts, &ui_ctx);
	cli->ui_ctx = &ui_ctx;

	size_t i;
	for (i = 0; i < input_mode_count; ++i) {
		keymap_init(&cli->keymaps[i]);
	}

	if (!parse_keymap(cli->keymaps, &ui_ctx)) {
		return false;
	}

	client_reset_input(cli);


	cli->im = im_normal;
	cli->ckm = &cli->keymaps[cli->im];

	cli->next_act.type = at_move;

#ifndef NDEBUG
	darr_init(&cli->debug_path.path_points, sizeof(struct point));
#endif

	switch (opts->mode) {
	case client_mode_map_viewer:
		load_world_from_path(opts->load_map, &cli->world->chunks);
		cli->tick = tick_basic;
		break;
	case client_mode_offline:
		if (!init_offline(cli)) {
			return false;
		}
		break;
	case client_mode_online:
		if (!init_online(cli, opts)) {
			return false;
		}
		break;
	}

	if (opts->cmds) {
		/* HACK: this is because the first time render is called, the
		 * opengl ui overwrites the camera position making goto
		 * commands ineffective */
		ui_render(cli);
		run_cmd_string(cli, opts->cmds);
	}

	L("client initialized");
	return true;
}
