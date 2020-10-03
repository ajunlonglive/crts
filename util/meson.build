if get_option('build-extras')
	subcmds += 'extras'

	executable(
		'snap',
		dependencies: [libm],
		include_directories: shared_inc,
		install: true,
		install_dir: crts_libexecdir,
		sources: files([
			'../shared/math/geom.c',
			'../shared/serialize/base.c',
			'../shared/serialize/chunk.c',
			'../shared/serialize/coder.c',
			'../shared/serialize/to_disk.c',
			'../shared/sim/chunk.c',
			'../shared/types/darr.c',
			'../shared/types/hash.c',
			'../shared/types/hdarr.c',
			'../shared/util/file_formats/tga.c',
			'../shared/util/log.c',
			'../shared/util/mem.c',
			'snap.c'
		])
	)
endif

if get_option('build-launcher')
	abs_subcmd_dir = join_paths(get_option('prefix'), get_option('subcmd-dir'))

	subcmd_def = []
	foreach cmd : subcmds
		subcmd_def += '-DSUBCMD_' + cmd
	endforeach

	executable(
		'crts',
		sources: files(['launcher.c']),
		include_directories: shared_inc,
		c_args: global_flags + ['-DLIBEXECDIR="' + abs_subcmd_dir + '"', subcmd_def],
		install: true
	)
endif
