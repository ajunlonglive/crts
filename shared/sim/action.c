#include <string.h>

#include "shared/constants/globals.h"
#include "shared/sim/action.h"
#include "shared/types/geom.h"
#include "shared/util/log.h"

void
action_init(struct action *act)
{
	memset(act, 0, sizeof(struct action));
}

void
action_inspect(struct action *act)
{
	L(
		"action type: %d\n"
#ifdef CRTS_SERVER
		"  id: %u owner: %u\n"
		"  workers (r: %u, a: %u)\n"
#endif
		"  range: (%d, %d) r: %d",
		act->type,
#ifdef CRTS_SERVER
		act->id,
		act->motivator,
		act->workers_requested,
		act->workers_assigned,
#endif
		act->range.center.x,
		act->range.center.y,
		act->range.r
		);
}
