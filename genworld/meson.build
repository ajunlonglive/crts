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
	'../shared/opengl/window.c',
	'gen.c',
	'main.c',
	'opts.c',
])

genworld_flags = []

if libglfw.found()

	deps += cc.find_library('pthread', required: true, static: get_option('static'))

	deps += libglfw

	if libwayland_client.found()
		deps += libwayland_client
	endif

	genworld_flags += '-DOPENGL_UI'

	src += files([
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
		'gl.c',
		'render_terrain.c',
		'worker.c',
	])

	src += font_atlas[1]
	genworld_flags += '-DINCLUDE_FONT_ATLAS'
endif

executable(
	'genworld',
	include_directories: shared_inc,
	sources: src,
	dependencies: deps,
	install: true,
	c_args: genworld_flags,
	link_args: genworld_flags
)
