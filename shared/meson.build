shared_srcs = files([
	#'sim/alignment.c',
	#'net/ack.c',
	#'net/bind_sock.c',
	#'net/connection.c',
	#'net/defs.c',
	#'net/msg_queue.c',
	#'net/net_ctx.c',
	#'net/pool.c',
	#'net/recv_msgs.c',
	#'net/send_msgs.c',
	'constants/globals.c',
	'math/delaunay.c',
	'math/geom.c',
	'math/hash.c',
	'math/kernel_filter.c',
	'math/linalg.c',
	'math/perlin.c',
	'math/rand.c',
	'math/triangle.c',
	'math/trigraph.c',
	'msgr/msgr.c',
	'msgr/transport/basic.c',
	'pathfind/abstract.c',
	'pathfind/api.c',
	'pathfind/local.c',
	'pathfind/meander.c',
	'pathfind/preprocess.c',
	'serialize/base.c',
	'serialize/chunk.c',
	'serialize/coder.c',
	'serialize/message.c',
	'serialize/net.c',
	'serialize/to_disk.c',
	'sim/action.c',
	'sim/chunk.c',
	'sim/ent.c',
	'sim/ent_buckets.c',
	'sim/tiles.c',
	'sim/world.c',
	'types/bheap.c',
	'types/darr.c',
	'types/hash.c',
	'types/hdarr.c',
	'util/assets.c',
	'util/file_formats/load_tga.c',
	'util/file_formats/tga.c',
	'util/inih.c',
	'util/log.c',
	'util/mem.c',
	'util/text.c',
	'util/time.c',
	'util/util.c',
]) + [configure_file(
	configuration: version_info,
	input: files('version.c.in'),
	output: 'version.c'
)]

libcrts_shared = library(
	'crts_shared',
	include_directories: shared_inc,
	install: false,
	sources: shared_srcs,
)

if get_option('opengl-ui')
	gl_shared_deps = [libm, libglfw]

	gl_shared_srcs = [font_atlas[1]] + files([
		'opengl/glad/src/gl.c',
		'opengl/loaders/obj.c',
		'opengl/loaders/shader.c',
		'opengl/render/text.c',
		'opengl/shader.c',
		'opengl/util.c',
		'opengl/window.c',
	])

	libcrts_gl_shared = library(
		'crts_gl_shared',
		c_args: global_flags,
		dependencies: gl_shared_deps,
		include_directories: shared_inc,
		install: false,
		sources: gl_shared_srcs,
	)
endif
