src = files(
    'gen/erosion.c',
    'gen/faults.c',
    'gen/filters.c',
    'gen/gen.c',
    'gen/opts.c',
    'gen/rasterize.c',
    'gen/write_tiles.c',
    'terragen.c',
)

flags = global_flags
dependencies = crts_dependencies

#if 'opengl' in get_option('uis')
#	src += files([
#		'opengl/render/menu.c',
#		'opengl/render/mesh.c',
#		'opengl/render/pixels.c',
#		'opengl/ui.c',
#		'opengl/worker.c',
#	])
#
#	dependencies += [libglfw]
#endif

crts_link_with += library(
    'terragen',
    c_args: flags,
    dependencies: dependencies,
    include_directories: include,
    sources: src,
)
