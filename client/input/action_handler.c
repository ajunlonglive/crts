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

	cli->action = id;
}
