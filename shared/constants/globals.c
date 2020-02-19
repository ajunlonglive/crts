#include "shared/constants/action_info.h"
#include "shared/sim/action.h"

const struct action_info ACTIONS[] = {
	/*                  name         maxw   minw   diff.  satis.  */
	[at_none]       = { "nothing",      0,     0,     0,      0 },
	[at_move]       = { "move",      9999,     0,     1,    100 },
	[at_harvest]    = { "harvest",   9999,     0,    25,    100 },
};
