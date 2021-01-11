subdir('gen')

if 'opengl' in get_option('uis')
	src = files([
		'opengl/render/menu.c',
		'opengl/render/mesh.c',
		'opengl/render/pixels.c',
		'opengl/ui.c',
		'opengl/worker.c',
	])

	libthreads = dependency('threads')

	crts_dependencies += libthreads
	crts_link_with += library(
		'terragen_interactive',
		c_args: global_flags,
		dependencies: [libm, libglfw, libthreads],
		include_directories: include,
		sources: src,
	)
endif
