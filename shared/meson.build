src = files([
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
	'msgr/transport/rudp/packet.c',
	'msgr/transport/rudp/queue.c',
	'msgr/transport/rudp/recv.c',
	'msgr/transport/rudp/send.c',
	'msgr/transport/rudp/seq_buf.c',
	'msgr/transport/rudp/util.c',
	'pathfind/abstract.c',
	'pathfind/api.c',
	'pathfind/local.c',
	'pathfind/preprocess.c',
	'platform/common/sockets.c',
	'serialize/base.c',
	'serialize/byte_swappers.c',
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
	'sound/sound.c',
	'types/bheap.c',
	'types/darr.c',
	'types/hash.c',
	'types/hdarr.c',
	'types/sack.c',
	'util/assets.c',
	'util/file_formats/load_tga.c',
	'util/file_formats/tga.c',
	'util/file_formats/wav.c',
	'util/inih.c',
	'util/log.c',
	'util/mem.c',
	'util/text.c',
	'util/timer.c',
	'util/util.c',
]) + [configure_file(
	configuration: version_info,
	input: files('version.c.in'),
	output: 'version.c'
)]

deps = []

subdir('platform/' + platform)

if get_option('tracy')
	src += files(get_option('tracy_dir') + '/TracyClient.cpp')
endif

if need_ui
	if 'opengl' in get_option('uis')
		src += [font_atlas[1]] + files([
			'opengl/glad/src/gl.c',
			'opengl/loaders/obj.c',
			'opengl/loaders/shader.c',
			'opengl/menu.c',
			'opengl/render/shapes.c',
			'opengl/render/text.c',
			'opengl/shader.c',
			'opengl/util.c',
			'opengl/window.c',
		])

		deps += libglfw
	endif

	if get_option('sound')
		src += files([
			'sound/core.c',
			'sound/setup.c',
		])

		deps += libsoundio
	endif
endif

libcrts_shared = library(
	'crts_shared',
	c_args: global_flags,
	cpp_args: global_flags,
	dependencies: deps,
	include_directories: include,
	sources: src,
)

crts_link_with += libcrts_shared
