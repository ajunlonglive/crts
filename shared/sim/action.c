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
#ifdef CRTS_SERVER
	L(
		"action type: %d\n"
		"  id: %u owner: %u\n"
		"  workers (r: %u, a: %u)\n"
		"  range: (%d, %d) r: %d",
		act->type,
		act->id,
		act->motivator,
		act->workers_requested,
		act->workers_assigned,
		act->range.center.x,
		act->range.center.y,
		act->range.r
		);
#else
	L(
		"action type: %d\n"
		"  range: (%d, %d) r: %d",
		act->type,
		act->range.center.x,
		act->range.center.y,
		act->range.r
		);
#endif
}
