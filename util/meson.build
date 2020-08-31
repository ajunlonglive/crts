if get_option('build-macos-app')
	abs_libexecdir = '.'
else
	abs_libexecdir = get_option('prefix') + '/' + crts_libexecdir
endif
executable(
	'crts',
	sources: files(['launcher.c']),
	include_directories: shared_inc,
	c_args: global_flags + ['-DLIBEXECDIR="' + abs_libexecdir  + '"'],
	install: true
)

executable(
	'snap',
	dependencies: [libm],
	include_directories: shared_inc,
	install: true,
	install_dir: crts_libexecdir,
	sources: files([
		'../shared/math/geom.c',
		'../shared/serialize/coder.c',
		'../shared/serialize/to_disk.c',
		'../shared/serialize/world.c',
		'../shared/sim/chunk.c',
		'../shared/types/darr.c',
		'../shared/types/hash.c',
		'../shared/types/hdarr.c',
		'../shared/util/log.c',
		'../shared/util/mem.c',
		'snap.c'
	])
)
