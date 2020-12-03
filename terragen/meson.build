src = files([
	'../shared/constants/globals.c',
	'../shared/math/delaunay.c',
	'../shared/math/geom.c',
	'../shared/math/hash.c',
	'../shared/math/kernel_filter.c',
	'../shared/math/linalg.c',
	'../shared/math/perlin.c',
	'../shared/math/rand.c',
	'../shared/math/triangle.c',
	'../shared/math/trigraph.c',
	'../shared/serialize/base.c',
	'../shared/serialize/chunk.c',
	'../shared/serialize/coder.c',
	'../shared/serialize/to_disk.c',
	'../shared/sim/chunk.c',
	'../shared/sim/ent.c',
	'../shared/sim/world.c',
	'../shared/sim/world.c',
	'../shared/types/darr.c',
	'../shared/types/hash.c',
	'../shared/types/hdarr.c',
	'../shared/util/assets.c',
	'../shared/util/inih.c',
	'../shared/util/log.c',
	'../shared/util/mem.c',
	'../shared/util/text.c',
	'../shared/util/util.c',
	'gen/erosion.c',
	'gen/faults.c',
	'gen/filters.c',
	'gen/gen.c',
	'gen/gen.c',
	'gen/opts.c',
	'gen/rasterize.c',
	'gen/write_tiles.c',
	'main.c',
]) + [
	configure_file(configuration: version_info,
		input: files('../include/version.h'), output: 'version.h')
]

deps = [libm]

flags = global_flags

if get_option('opengl-ui')
	src += gl_src + files([
		'../shared/util/time.c',
		'opengl/render/menu.c',
		'opengl/render/mesh.c',
		'opengl/render/pixels.c',
		'opengl/ui.c',
		'opengl/worker.c',
	])

	deps += cc.find_library('pthread', static: get_option('static'))
	deps += gl_deps

	flags += gl_flags
endif

if terragen_assets['static'].length() > 0
	# Embedded assets
	if get_option('embed-assets')
		src += custom_target(
			'embedded_data.h',
			input: terragen_assets['static'] + terragen_assets['gen'],
			output: 'embedded_data.h',
			capture: true,
			command: [embed_binary] + terragen_assets['static'] + terragen_assets['gen_path'],
			build_by_default: true
		)
		flags += '-DINCLUDE_EMBEDDED_DATA'
	else
		assets_to_install += terragen_assets['static']
	endif

	# Export manifest
	src += custom_target(
		'asset manifest',
		output: 'asset_manifest.h',
		capture: true,
		command: [export_manifest] + terragen_assets['static'] + terragen_assets['gen_path'],
		build_by_default: true
	)
	flags += '-DINCLUDE_EXPORTED_MANIFEST'
endif

executable(
	'terragen',
	include_directories: shared_inc,
	sources: src,
	dependencies: deps,
	install: true,
	c_args: flags,
	link_args: flags,
	install_dir: get_option('subcmd-dir')
)
