flags = global_flags
foreach target: get_option('targets')
	flags += '-DCRTS_HAVE_' + target
endforeach

src = files([
	'assets.c',
	'launcher.c',
	'opts.c',
])

if assets_static.length() > 0
	# Embedded assets
	if get_option('embed_assets')
		src += custom_target(
			'embedded_data.h',
			input: assets_static + assets_gen,
			output: 'embedded_data.h',
			capture: true,
			command: [embed_binary] + assets_static + assets_gen_path,
		)
		flags += '-DINCLUDE_EMBEDDED_DATA'
	endif

	# Export manifest
	src += custom_target(
		'asset manifest',
		output: 'asset_manifest.h',
		capture: true,
		command: [export_manifest] + assets_static + assets_gen_path,
	)
endif

crts_exe = executable(
	'crts',
	c_args: flags,
	dependencies: crts_dependencies,
	include_directories: include,
	install: get_option('packaging') == 'unix',
	link_with: crts_link_with,
	sources: src,
)
