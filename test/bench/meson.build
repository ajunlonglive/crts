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
