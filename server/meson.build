src = files([
	'aggregate_msgs.c',
	'handle_msg.c',
	'opts.c',
	'server.c',
	'sim/ai.c',
	'sim/ent.c',
	'sim/ent_lookup.c',
	'sim/environment.c',
	'sim/sim.c',
	'sim/update_tile.c',
])

crts_link_with += library(
	'crts_server',
	c_args: global_flags,
	dependencies: [],
	include_directories: include,
	sources: src,
)
