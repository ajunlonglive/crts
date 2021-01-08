tests = [
	{ 'src': 'path.c' },
	{ 'src': 'hdarr.c' },
	{ 'src': 'hash.c' },
	{ 'src': 'readlines.c', 'args': [meson.source_root() + '/meson.build'] },
	{ 'src': 'inih.c' },
	{ 'src': 'shrink.c' },
	{ 'src': 'save.c' },
	{ 'src': 'random.c', 'build_only': true },
	{ 'src': 'blur.c', 'build_only': true },
	{ 'src': 'heap.c' },
	{ 'src': 'sack.c' },
	{ 'src': 'message_sack.c' },
]

foreach t: tests
	src = t['src']
	name = src.split('.')[0]
	args = []

	if t.has_key('args')
		args = t['args']
	endif

	test_exe = executable(
		name,
		dependencies: [libm],
		include_directories: shared_inc,
		install: false,
		link_with: [libcrts_shared],
		sources: [src],
	)

	if not (t.has_key('build_only') and t['build_only'])
		test(name, test_exe, args: args)
	endif
endforeach

subdir('bench')
