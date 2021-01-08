src = files([
	#'net/ack.c',
	#'net/bind_sock.c',
	#'net/connection.c',
	#'net/defs.c',
	#'net/msg_queue.c',
	#'net/net_ctx.c',
	#'net/pool.c',
	#'net/recv_msgs.c',
	#'net/send_msgs.c',
	#'sim/alignment.c',
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
	'msgr/transport/rudp.c',
	'msgr/transport/rudp/cx_pool.c',
	'pathfind/abstract.c',
	'pathfind/api.c',
	'pathfind/local.c',
	'pathfind/meander.c',
	'pathfind/preprocess.c',
	'platform/sockets/berkeley.c',
	'platform/sockets/common.c',
	'platform/sockets/dummy.c',
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
	'types/sack.c',
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

if get_option('tracy')
	src += files(get_option('tracy_dir') + '/TracyClient.cpp')
endif

libcrts_shared = library(
	'crts_shared',
	c_args: global_flags,
	cpp_args: global_flags,
	dependencies: [libm],
	include_directories: include,
	install: false,
	sources: src,
)

crts_link_with += libcrts_shared

if 'opengl' in get_option('uis')
	src = [font_atlas[1]] + files([
		'opengl/glad/src/gl.c',
		'opengl/loaders/obj.c',
		'opengl/loaders/shader.c',
		'opengl/render/text.c',
		'opengl/shader.c',
		'opengl/util.c',
		'opengl/window.c',
	])

	crts_link_with += library(
		'crts_gl_shared',
		c_args: global_flags,
		dependencies: [libm, libglfw],
		include_directories: include,
		install: false,
		sources: src,
	)

	crts_dependencies += [libm, libglfw]
endif
