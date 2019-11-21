#include "geom.h"
#include "action.h"

void action_init(struct action *act)
{
	act->completion = 0;
	act->workers = 0;
	act->id = 0;
	act->motivator = 0;
	act->type = 0;

	act->range.center.x = 0;
	act->range.center.y = 0;
	act->range.r = 0;
}
