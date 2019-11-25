#include "geom.h"
#include "action.h"
#include "log.h"
#include "globals.h"

void action_init(struct action *act)
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

void action_inspect(struct action *act)
{
	L("action #%d m: %d | %3d%%, w: %d | (%d, %d) r: %d",
	  act->id,
	  act->motivator,
	  (act->completion * 100) / ACTIONS[act->type].completed_at,
	  act->workers,
	  act->range.center.x,
	  act->range.center.y,
	  act->range.r
	  );
}
