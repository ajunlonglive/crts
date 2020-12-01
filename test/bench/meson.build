hash_src = files([
	'../../shared/math/hash.c',
	'../../shared/types/darr.c',
	'../../shared/types/hash.c',
	'../../shared/util/log.c',
	'../../shared/util/mem.c',
	'hash.c',
])
hash = executable('hash', c_args: ['-DCRTS_HASH_STATS'],
	include_directories: shared_inc, sources: hash_src, install: false)
benchmark('hash', hash)

path_local_src = files([
	'../../shared/constants/globals.c',
	'../../shared/math/geom.c',
	'../../shared/math/hash.c',
	'../../shared/pathfind/abstract.c',
	'../../shared/pathfind/api.c',
	'../../shared/pathfind/local.c',
	'../../shared/pathfind/preprocess.c',
	'../../shared/sim/chunk.c',
	'../../shared/sim/tiles.c',
	'../../shared/types/bheap.c',
	'../../shared/types/darr.c',
	'../../shared/types/hash.c',
	'../../shared/types/hdarr.c',
	'../../shared/util/log.c',
	'../../shared/util/mem.c',
	'path_local.c',
])
path_local = executable('path_local', c_args: ['-DCRTS_PATHFINDING'],
	include_directories: shared_inc, sources: path_local_src, install: false)
benchmark('path_local', path_local)
