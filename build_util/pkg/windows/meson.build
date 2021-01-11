if get_option('upload')
	dist_file = custom_target(
		'crts_windows.zip',
		input: crts_exe,
		output: 'crts_windows.zip',
		command: [zip, '@OUTDIR@/crts_windows.zip', crts_exe, '/bin/cygwin1.dll'],
		build_by_default: true,
	)
endif
