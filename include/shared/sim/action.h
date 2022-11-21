#ifndef SHARED_SIM_ACTION_H
#define SHARED_SIM_ACTION_H

enum action {
	act_neutral,
	act_create,
	act_destroy,
	act_terrain,
	action_count,
};

enum act_terrain_arg {
	act_terrain_raise,
	act_terrain_lower,
	act_terrain_flatten,
	act_terrain_arg_count,
};
#endif
