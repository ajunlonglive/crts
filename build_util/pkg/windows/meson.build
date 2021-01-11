if get_option('upload')
	dist_filename =	'crts_windows-' + version_info['version'] + '.zip'
	dist_file = custom_target(
		dist_filename,
		input: crts_exe,
		output: dist_filename,
		command: [zip, '@OUTDIR@/' + dist_filename, crts_exe, '/bin/cygwin1.dll'],
		build_by_default: true,
	)
endif
