path_src = files([
	'../server/sim/pathfind/heap.c',
	'../server/sim/pathfind/meander.c',
	'../server/sim/pathfind/pathfind.c',
	'../server/sim/pathfind/pg_node.c',
	'../server/sim/pathfind/pgraph.c',
	'../shared/constants/globals.c',
	'../shared/math/geom.c',
	'../shared/math/perlin.c',
	'../shared/math/rand.c',
	'../shared/sim/alignment.c',
	'../shared/sim/chunk.c',
	'../shared/sim/ent.c',
	'../shared/sim/tiles.c',
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

shrink_src = files(['../shared/serialize/coder.c', 'shrink.c'])
shrink = executable('shrink', include_directories: shared_inc, sources: shrink_src,
	dependencies: [libm], install: false)
test('shrink', shrink)

save_src = files(['../shared/math/geom.c', '../shared/serialize/coder.c',
	'../shared/serialize/world.c', '../shared/sim/chunk.c',
	'../shared/types/darr.c', '../shared/types/hash.c',
	'../shared/types/hdarr.c', '../shared/util/log.c',
	'../shared/util/mem.c', 'save.c'])
save = executable('save', include_directories: shared_inc, sources: save_src,
	dependencies: [libm], install: false)
test('save', save)
