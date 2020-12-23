if get_option('build-extras')
	subcmds += 'extras'

	executable(
		'snap',
		dependencies: [libm],
		include_directories: shared_inc,
		install: true,
		install_dir: get_option('subcmd-dir'),
		link_with: libcrts_shared,
		sources: files(['snap.c']),

	)
endif

if get_option('build-launcher')
	abs_subcmd_dir = join_paths(get_option('prefix'), get_option('subcmd-dir'))

	subcmd_def = []
	foreach cmd : subcmds
		subcmd_def += '-DSUBCMD_' + cmd
	endforeach

	libexecdir_flag = '-DLIBEXECDIR="' + abs_subcmd_dir + '"'

	executable(
		'crts',
		sources: files(['launcher.c']),
		include_directories: shared_inc,
		c_args: global_flags + [libexecdir_flag, subcmd_def],
		install: true
	)
endif
