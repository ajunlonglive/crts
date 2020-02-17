#include <string.h>

#include "shared/constants/action_info.h"
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
		"  id: %u owner: %u\n"
		"  workers (r: %u, a: %u, ir: %u)\n"
		"  range: (%d, %d) r: %d",
		act->type,
		act->id,
		act->motivator,
		act->workers.requested,
		act->workers.assigned,
		act->workers.in_range,
		act->range.center.x,
		act->range.center.y,
		act->range.r
		);
}
