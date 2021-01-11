src = files([
	'aggregate_msgs.c',
	'handle_msg.c',
	'opts.c',
	'server.c',
	'sim/action.c',
	'sim/do_action.c',
	'sim/do_action/build.c',
	'sim/do_action/carry.c',
	'sim/do_action/fight.c',
	'sim/do_action/harvest.c',
	'sim/do_action/move.c',
	'sim/ent.c',
	'sim/ent_lookup.c',
	'sim/environment.c',
	'sim/sim.c',
	'sim/storehouse.c',
	'sim/update_tile.c',
	'sim/worker.c',
])

crts_link_with += library(
	'crts_server',
	c_args: global_flags,
	dependencies: [libm],
	include_directories: include,
	sources: src,
)
