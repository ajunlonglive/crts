make_windows = find_program('./make_windows')
name = 'crts_windows'

binaries = [crts_exe]

resources_gen = assets_static + assets_gen
resources = assets_static + assets_gen_path

crts_windows = custom_target(
	name,
	input: binaries + resources_gen,
	output: name,
	command: [
		make_windows,
		'@OUTDIR@/' + name,
		'-b', binaries,
		'-r', resources
	],
	build_by_default: true,
)

if get_option('upload')
	dist_filename =	name + '-' + version_info['version'] + '.zip'
	dist_file = custom_target(
		dist_filename,
		input: crts_windows,
		output: dist_filename,
		command: [zip, '@OUTDIR@/' + dist_filename, '@INPUT@'],
		build_by_default: true,
	)
endif
