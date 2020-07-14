src = files([
	'../shared/constants/globals.c',
	'../shared/math/geom.c',
	'../shared/math/rand.c',
	'../shared/messaging/client_message.c',
	'../shared/net/ack.c',
	'../shared/net/bind_sock.c',
	'../shared/net/connection.c',
	'../shared/net/defs.c',
	'../shared/net/inet_aton.c',
	'../shared/net/msg_queue.c',
	'../shared/net/net_ctx.c',
	'../shared/net/pool.c',
	'../shared/net/recv_msgs.c',
	'../shared/net/send_msgs.c',
	'../shared/serialize/base.c',
	'../shared/serialize/client_message.c',
	'../shared/serialize/geom.c',
	'../shared/serialize/net.c',
	'../shared/serialize/server_message.c',
	'../shared/sim/action.c',
	'../shared/sim/chunk.c',
	'../shared/sim/ent.c',
	'../shared/sim/world.c',
	'../shared/types/darr.c',
	'../shared/types/hash.c',
	'../shared/types/hdarr.c',
	'../shared/util/assets.c',
	'../shared/util/inih.c',
	'../shared/util/log.c',
	'../shared/util/mem.c',
	'../shared/util/text.c',
	'../shared/util/time.c',
	'../shared/util/util.c',
	'cfg/common.c',
	'cfg/graphics.c',
	'cfg/keymap.c',
	'hiface.c',
	'input/action_handler.c',
	'input/cmdline.c',
	'input/handler.c',
	'input/keymap.c',
	'input/move_handler.c',
	'main.c',
	'net.c',
	'opts.c',
	'request_missing_chunks.c',
	'ui/common.c',
	'world_update.c',
])
deps = []
flags = global_flags + []

src += configure_file(configuration: version_info,
	input: files('../include/version.h'), output: 'version.h')

libglfw = dependency('glfw3', required: get_option('opengl-ui'),
	static: get_option('static'))
libncurses = dependency('ncurses', required: get_option('ncurses-ui'),
	static: get_option('static'))
libwayland_client = dependency('wayland-client',
	required: get_option('opengl-ui-wayland'),
	static: get_option('static'))

# UIs

if libncurses.found()
	src += files([
		'ui/ncurses/container.c',
		'ui/ncurses/graphics.c',
		'ui/ncurses/info.c',
		'ui/ncurses/ui.c',
		'ui/ncurses/window.c',
		'ui/ncurses/world.c',
	])

	flags += '-DNCURSES_UI'

	deps += libncurses
endif

if libglfw.found()
	src += files([
		'../lib/glad/src/gl.c',
		'../shared/math/linalg.c',
		'cfg/opengl.c',
		'ui/opengl/globals.c',
		'ui/opengl/input.c',
		'ui/opengl/loaders/color_cfg.c',
		'ui/opengl/loaders/obj.c',
		'ui/opengl/loaders/shader.c',
		'ui/opengl/loaders/tga.c',
		'ui/opengl/render.c',
		'ui/opengl/render/chunks.c',
		'ui/opengl/render/ents.c',
		'ui/opengl/render/hud.c',
		'ui/opengl/render/selection.c',
		'ui/opengl/render/shadows.c',
		'ui/opengl/render/text.c',
		'ui/opengl/render/water.c',
		'ui/opengl/shader.c',
		'ui/opengl/shader_multi_obj.c',
		'ui/opengl/ui.c',
		'ui/opengl/util.c',
		'ui/opengl/window.c',
	])

	flags += '-DOPENGL_UI'

	deps += [libm, libglfw]
	if libwayland_client.found()
		deps += libwayland_client
	endif

	src += font_atlas[1]
	flags += '-DINCLUDE_FONT_ATLAS'
	assets += font_atlas

	assets += obj_assets

	assets += [
		meson.source_root() + '/assets/cfg/opengl.ini',
		meson.source_root() + '/assets/water.tga'
	]

	subdir('ui/opengl/shaders')
endif

assets += [
	meson.source_root() + '/assets/cfg/keymap.ini',
	meson.source_root() + '/assets/cfg/graphics.ini',
]

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

crts_flags = flags

executable(
	'crts',
	include_directories: shared_inc,
	sources: src,
	dependencies: deps,
	install: true,
	c_args: crts_flags,
	link_args: crts_flags
)
