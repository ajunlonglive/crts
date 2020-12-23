benchmarks = [
	{ 'src': 'hash.c', 'flags': ['-DCRTS_HASH_STATS'] },
	{ 'src': 'path_local.c' },
]

foreach b: benchmarks
	src = b['src']
	name = src.split('.')[0]
	flags = []

	if b.has_key('flags')
		flags = b['flags']
	endif

	benchmark(
		name,
		executable(
			name,
			c_args: flags,
			dependencies: [libm],
			include_directories: shared_inc,
			install: false,
			link_with: [libcrts_shared],
			sources: [src],
		),
	)
endforeach
