src = files([
	'src/handle_msg.c',
	'src/main.c',
	'src/net/connection.c',
	'src/net/pool.c',
	'src/net/receive.c',
	'src/net/respond.c',
	'src/net/server.c',
	'src/opts.c',
	'src/sim/sim.c',
	'src/sim/terrain.c',
	'src/traps.c',
	'../shared/sim/world.c',
	'../shared/sim/chunk.c',
	'../shared/sim/ent.c',
	'../shared/sim/alignment.c',
	'../shared/sim/action.c',
	'../shared/types/queue.c',
	'../shared/types/hash.c',
	'../shared/messaging/server_message.c',
	'../shared/serialize/client_message.c',
	'../shared/serialize/server_message.c',
	'../shared/serialize/base.c',
	'../shared/serialize/geom.c',
	'../shared/math/geom.c',
	'../shared/math/perlin.c',
	'../shared/constants/action_info.c',
])

deps = [
	cc.find_library('pthread', required : true),
	cc.find_library('m', required : true),
]

executable('crtcd', include_directories: shared_inc, sources: src, dependencies: deps, install: true)
