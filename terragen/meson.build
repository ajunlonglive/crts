src = files([
	'gen/erosion.c',
	'gen/faults.c',
	'gen/filters.c',
	'gen/gen.c',
	'gen/opts.c',
	'gen/rasterize.c',
	'gen/write_tiles.c',
	'terragen.c',
])

flags = global_flags
dependencies = []

if 'opengl' in get_option('uis')
	src += files([
		'opengl/render/menu.c',
		'opengl/render/mesh.c',
		'opengl/render/pixels.c',
		'opengl/ui.c',
		'opengl/worker.c',
	])

	libthreads = dependency('threads')
	crts_dependencies += libthreads
	dependencies += [libthreads, libglfw]
endif

crts_link_with += library(
	'terragen',
	c_args: flags,
	dependencies: dependencies,
	include_directories: include,
	sources: src,
)
