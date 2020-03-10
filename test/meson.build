path_src = files([
	'../server/sim/pathfind/heap.c',
	'../server/sim/pathfind/meander.c',
	'../server/sim/pathfind/pathfind.c',
	'../server/sim/pathfind/pg_node.c',
	'../server/sim/pathfind/pgraph.c',
	'../server/sim/terrain.c',
	'../shared/math/geom.c',
	'../shared/math/perlin.c',
	'../shared/sim/alignment.c',
	'../shared/sim/chunk.c',
	'../shared/sim/ent.c',
	'../shared/sim/world.c',
	'../shared/types/darr.c',
	'../shared/types/hash.c',
	'../shared/types/hdarr.c',
	'../shared/util/log.c',
	'../shared/util/mem.c',
	'path.c',
])

path_deps = [
	cc.find_library('m', required : true),
]

hdarr_src = files([
	'../shared/types/darr.c',
	'../shared/types/hash.c',
	'../shared/types/hdarr.c',
	'../shared/util/log.c',
	'../shared/util/mem.c',
	'hdarr.c',
])

server_flags = ['-DCRTS_SERVER']

path = executable('path', c_args: server_flags, include_directories: shared_inc,
	sources: path_src, dependencies: path_deps, install: false)
test('pathfinding', path)

hash = executable('hash', include_directories: shared_inc,
	sources: files(['hash.c', '../shared/types/hash.c']), install: false)
test('hash', path)

hdarr = executable('hdarr', include_directories: shared_inc, sources: hdarr_src,
	install: false)
test('hdarr', path)

