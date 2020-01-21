#include "action.h"
#include "constants/action_info.h"
#include "types/geom.h"
#include "util/log.h"

void
action_init(struct action *act)
{
	act->completion = 0;
	act->workers = 0;
	act->workers_in_range = 0;
	act->id = 0;
	act->motivator = 0;
	act->type = 0;

	act->range.center.x = 0;
	act->range.center.y = 0;
	act->range.r = 0;
}

void
action_inspect(struct action *act)
{
	L("action type: %s\n  id: %ld owner: %d\n  %3d%% completed, %d workers (%d in range)\n  range: (%d, %d) r: %d",
	  ACTIONS[act->type].name,
	  act->id,
	  act->motivator,
	  (act->completion * 100) / ACTIONS[act->type].completed_at,
	  act->workers,
	  act->workers_in_range,
	  act->range.center.x,
	  act->range.center.y,
	  act->range.r
	  );
}
