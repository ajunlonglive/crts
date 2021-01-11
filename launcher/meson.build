flags = global_flags
foreach target: get_option('targets')
	flags += '-DCRTS_HAVE_' + target
endforeach

src = files([
	'opts.c',
	'assets.c',
	'launcher.c',
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
	else
		assets_to_install += assets_static
	endif

	# Export manifest
	src += custom_target(
		'asset manifest',
		output: 'asset_manifest.h',
		capture: true,
		command: [export_manifest] + assets_static + assets_gen_path,
	)
endif

executable(
	'crts',
	c_args: flags,
	dependencies: crts_dependencies,
	include_directories: include,
	install: true,
	link_with: crts_link_with,
	sources: src,
)
