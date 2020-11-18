path_src = files([
	'../shared/constants/globals.c',
	'../shared/math/geom.c',
	'../shared/math/hash.c',
	'../shared/pathfind/abstract.c',
	'../shared/pathfind/api.c',
	'../shared/pathfind/local.c',
	'../shared/pathfind/preprocess.c',
	'../shared/sim/chunk.c',
	'../shared/sim/ent.c',
	'../shared/sim/tiles.c',
	'../shared/sim/world.c',
	'../shared/types/bheap.c',
	'../shared/types/darr.c',
	'../shared/types/hash.c',
	'../shared/types/hdarr.c',
	'../shared/util/log.c',
	'../shared/util/mem.c',
	'path.c',
])

hdarr_src = files([
	'../shared/math/hash.c',
	'../shared/types/darr.c',
	'../shared/types/hash.c',
	'../shared/types/hdarr.c',
	'../shared/util/log.c',
	'../shared/util/mem.c',
	'hdarr.c',
])

hash_src = files([
	'../shared/math/hash.c',
	'../shared/types/darr.c',
	'../shared/types/hash.c',
	'../shared/util/log.c',
	'../shared/util/mem.c',
	'hash.c',
])

acker_src = files([
	'../shared/math/hash.c',
	'../shared/net/ack.c',
	'../shared/net/defs.c',
	'../shared/net/msg_queue.c',
	'../shared/serialize/base.c',
	'../shared/serialize/chunk.c',
	'../shared/serialize/coder.c',
	'../shared/serialize/message.c',
	'../shared/serialize/net.c',
	'../shared/types/darr.c',
	'../shared/types/hash.c',
	'../shared/util/log.c',
	'../shared/util/mem.c',
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

path = executable('path', c_args: ['-DCRTS_PATHFINDING'], include_directories: shared_inc,
	sources: path_src, dependencies: [libm], install: false)
test('pathfinding', path)

hash = executable('hash', include_directories: shared_inc, sources: hash_src,
	install: false)
test('hash', hash)

hdarr = executable('hdarr', include_directories: shared_inc, sources: hdarr_src,
	install: false)
test('hdarr', hdarr, timeout: 60)

acker = executable('acker', include_directories: shared_inc, sources: acker_src,
	install: false)
test('acker', acker)

readlines = executable('readlines', include_directories: shared_inc, sources: readlines_src,
	install: false)
test('readlines', readlines, args: [meson.source_root() + '/meson.build'])

inih = executable('inih', include_directories: shared_inc, sources: inih_src,
	install: false)
test('inih', inih )

shrink_src = files([
	'../shared/serialize/coder.c',
	'../shared/util/log.c',
	'shrink.c'
])

shrink = executable('shrink', include_directories: shared_inc, sources: shrink_src,
	dependencies: [libm], install: false)
test('shrink', shrink)

save_src = files([
	'../shared/math/geom.c',
	'../shared/math/hash.c',
	'../shared/serialize/base.c',
	'../shared/serialize/chunk.c',
	'../shared/serialize/coder.c',
	'../shared/sim/chunk.c',
	'../shared/types/darr.c',
	'../shared/types/hash.c',
	'../shared/types/hdarr.c',
	'../shared/util/log.c',
	'../shared/util/mem.c',
	'save.c'
])
save = executable('save', include_directories: shared_inc, sources: save_src,
	dependencies: [libm], install: false)
test('save', save)

random_src = files([
	'../shared/math/rand.c',
	'../shared/util/log.c',
	'random.c',
])
random = executable('random', include_directories: shared_inc, sources: random_src,
	dependencies: [libm], install: false)

blur_src = files([
	'../shared/math/kernel_filter.c',
	'../shared/opengl/loaders/tga.c',
	'../shared/util/assets.c',
	'../shared/util/file_formats/tga.c',
	'../shared/util/log.c',
	'blur.c',
])
blur = executable('blur', include_directories: shared_inc, sources: blur_src,
	dependencies: [libm], install: false)

heap_src = files([
	'../shared/math/rand.c',
	'../shared/types/bheap.c',
	'../shared/types/darr.c',
	'../shared/util/log.c',
	'../shared/util/mem.c',
	'heap.c',
])
heap = executable('heap', include_directories: shared_inc, sources: heap_src,
	dependencies: [libm], install: false)
test('heap', heap)

subdir('bench')
