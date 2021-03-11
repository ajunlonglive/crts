#include "posix.h"

#include "client/input/action_handler.h"
#include "client/input/helpers.h"
#include "shared/sim/action.h"
#include "shared/util/log.h"

void
set_action_type(struct client *cli)
{
	long id;

	if ((id = client_get_num(cli, 0)) >= action_count || id < 0) {
		return;
	}

	if (cli->keymap_describe) {
		cli_describe(cli, kmc_nav, "set_curs_action_type %d", id);
	}

	cli->action = id;
}
