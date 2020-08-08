path_src = files([
	'../server/sim/pathfind/heap.c',
	'../server/sim/pathfind/meander.c',
	'../server/sim/pathfind/pathfind.c',
	'../server/sim/pathfind/pg_node.c',
	'../server/sim/pathfind/pgraph.c',
	'../server/sim/terrain.c',
	'../shared/constants/globals.c',
	'../shared/math/geom.c',
	'../shared/math/perlin.c',
	'../shared/math/rand.c',
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

hdarr_src = files([
	'../shared/types/darr.c',
	'../shared/types/hash.c',
	'../shared/types/hdarr.c',
	'../shared/util/log.c',
	'../shared/util/mem.c',
	'hdarr.c',
])

hash_src = files([
	'../shared/types/darr.c',
	'../shared/types/hash.c',
	'../shared/util/log.c',
	'../shared/util/mem.c',
	'hash.c',
])

acker_src = files([
	'../shared/net/ack.c',
	'../shared/net/defs.c',
	'../shared/serialize/base.c',
	'../shared/serialize/net.c',
	'../shared/util/log.c',
	'acker.c',
])

readlines_src = files([
	'../shared/util/assets.c',
	'../shared/util/log.c',
	'../shared/util/text.c',
	'readlines.c',
])

inih_src = files([
	'../shared/util/assets.c',
	'../shared/util/log.c',
	'../shared/util/text.c',
	'../shared/util/inih.c',
	'inih.c',
])

#terrain_src = files([
#	'../genworld/gen.c',
#	'../shared/constants/globals.c',
#	'../shared/math/delaunay.c',
#	'../shared/math/geom.c',
#	'../shared/math/kernel_filter.c',
#	'../shared/math/linalg.c',
#	'../shared/math/perlin.c',
#	'../shared/math/rand.c',
#	'../shared/math/triangle.c',
#	'../shared/math/trigraph.c',
#	'../shared/sim/chunk.c',
#	'../shared/sim/ent.c',
#	'../shared/sim/world.c',
#	'../shared/sim/world.c',
#	'../shared/types/darr.c',
#	'../shared/types/hash.c',
#	'../shared/types/hdarr.c',
#	'../shared/util/log.c',
#	'../shared/util/mem.c',
#	'../shared/util/util.c',
#	'terrain.c',
#])

server_flags = ['-DCRTS_SERVER']

path = executable('path', c_args: server_flags, include_directories: shared_inc,
	sources: path_src, dependencies: [libm], install: false)
test('pathfinding', path)

hash = executable('hash', include_directories: shared_inc, sources: hash_src,
	install: false)
test('hash', hash)

hdarr = executable('hdarr', include_directories: shared_inc, sources: hdarr_src,
	install: false)
test('hdarr', hdarr)

acker = executable('acker', include_directories: shared_inc, sources: acker_src,
	install: false)
test('acker', acker)

readlines = executable('readlines', include_directories: shared_inc, sources: readlines_src,
	install: false)
test('readlines', readlines, args: [meson.source_root() + '/meson.build'])

inih = executable('inih', include_directories: shared_inc, sources: inih_src,
	install: false)
test('inih', inih )

#terrain = executable('terrain', c_args: server_flags, include_directories: shared_inc,
	#sources: terrain_src, install: false)
