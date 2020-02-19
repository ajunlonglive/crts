src = files([
	'../shared/constants/globals.c',
	'../shared/math/geom.c',
	'../shared/math/perlin.c',
	'../shared/messaging/server_message.c',
	'../shared/serialize/base.c',
	'../shared/serialize/client_message.c',
	'../shared/serialize/geom.c',
	'../shared/serialize/server_message.c',
	'../shared/sim/action.c',
	'../shared/sim/alignment.c',
	'../shared/sim/chunk.c',
	'../shared/sim/ent.c',
	'../shared/sim/world.c',
	'../shared/types/hash.c',
	'../shared/types/queue.c',
	'../shared/util/mem.c',
	'../shared/util/time.c',
	'handle_msg.c',
	'main.c',
	'net/connection.c',
	'net/pool.c',
	'net/receive.c',
	'net/respond.c',
	'net/server.c',
	'opts.c',
	'sim/action.c',
	'sim/do_action.c',
	'sim/pathfind/heap.c',
	'sim/pathfind/meander.c',
	'sim/pathfind/pathfind.c',
	'sim/pathfind/pg_node.c',
	'sim/pathfind/pgraph.c',
	'sim/sim.c',
	'sim/terrain.c',
	'sim/worker.c',
	'traps.c',
])

deps = [
	cc.find_library('m', required : true),
]

gheap_flags = '-DNDEBUG'
crtsd_flags = gheap_flags + ' -DCRTS_SERVER'

executable(
	'crtsd',
	include_directories: shared_inc,
	sources: src,
	dependencies: deps,
	install: true,
	c_args: crtsd_flags,
	link_args: crtsd_flags
)
