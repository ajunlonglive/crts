src = files([
	'../shared/constants/globals.c',
	'../shared/math/delaunay.c',
	'../shared/math/geom.c',
	'../shared/math/kernel_filter.c',
	'../shared/math/linalg.c',
	'../shared/math/perlin.c',
	'../shared/math/rand.c',
	'../shared/math/triangle.c',
	'../shared/math/trigraph.c',
	'../shared/opengl/window.c',
	'../shared/serialize/coder.c',
	'../shared/serialize/to_disk.c',
	'../shared/serialize/world.c',
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

deps = []

assets = []

flags = global_flags

if libglfw.found()
	src += gl_src + files([
		'../lib/glad/src/gl.c',
		'../shared/math/linalg.c',
		'../shared/opengl/loaders/obj.c',
		'../shared/opengl/loaders/shader.c',
		'../shared/opengl/loaders/tga.c',
		'../shared/opengl/render/text.c',
		'../shared/opengl/shader.c',
		'../shared/opengl/util.c',
		'../shared/opengl/window.c',
		'../shared/util/time.c',
		'opengl/render/menu.c',
		'opengl/render/mesh.c',
		'opengl/render/pixels.c',
		'opengl/ui.c',
		'opengl/worker.c',
	])

	deps += cc.find_library('pthread', static: get_option('static'))
	deps += gl_deps

	assets += [
		meson.source_root() + '/assets/cfg/opengl.ini',
		meson.source_root() + '/assets/water.tga',
		obj_assets
	]
	assets += terragen_shaders
	assets += gl_assets

	flags += gl_flags
endif

# Embedded assets
if get_option('embed-assets') and assets.length() > 0
	embedded_data = custom_target(
		'embedded_data.h',
		input: assets,
		output: 'embedded_data.h',
		capture: true,
		command: [embed_binary] + assets,
		build_by_default: true
	)

	flags += '-DINCLUDE_EMBEDDED_DATA'
	src += embedded_data
endif

# Export manifest
export_manifest = find_program(meson.source_root() + '/bin/export_manifest')
asset_manifest = custom_target(
	'asset manifest',
	output: 'asset_manifest.h',
	capture: true,
	command: [export_manifest] + assets,
	build_by_default: true
)
src += asset_manifest
flags += '-DINCLUDE_EXPORTED_MANIFEST'

executable('terragen', include_directories: shared_inc, sources: src,
	dependencies: deps, install: true, c_args: flags, link_args: flags)
