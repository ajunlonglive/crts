#include "action.h"
#include "globals.h"

const struct action_info ACTIONS[] = {
	/*                  name         maxw   minw   diff.  satis.  */
	[action_type_0] = { "action 1",   2,      1,    100,    100 },
	[action_type_1] = { "move",      20,     20,      1,    100 }
};
