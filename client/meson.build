src = files([
	'../shared/constants/blueprints.c',
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
	'cfg/common.c',
	'cfg/graphics.c',
	'cfg/keymap.c',
	'hiface.c',
	'input/action_handler.c',
	'input/handler.c',
	'input/keymap.c',
	'input/mouse.c',
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
embedded_assets = []
static_assets = []

src += configure_file(configuration: version_info,
	input: files('../include/version.h'), output: 'version.h')

# UIs

libncurses = dependency('ncurses', required: get_option('ncurses-ui'),
	static: get_option('static'))

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

libglfw = dependency('glfw3', required: get_option('opengl-ui'),
	static: get_option('static'))

if libglfw.found()
	src += files([
		'../lib/glad/src/gl.c',
		'../shared/math/linalg.c',
		'ui/opengl/color_cfg.c',
		'ui/opengl/globals.c',
		'ui/opengl/input.c',
		'ui/opengl/loaders/obj.c',
		'ui/opengl/loaders/tga.c',
		'ui/opengl/render.c',
		'ui/opengl/render/chunks.c',
		'ui/opengl/render/ents.c',
		'ui/opengl/render/hud.c',
		'ui/opengl/render/selection.c',
		'ui/opengl/render/text.c',
		'ui/opengl/ui.c',
		'ui/opengl/winutil.c',
	])

	flags += '-DOPENGL_UI'

	deps += [libm, libglfw]

	libwayland_client = dependency('wayland-client',
		required: get_option('opengl-ui-wayland'),
		static: get_option('static'))

	if libwayland_client.found()
		deps += libwayland_client
	endif

	# build assets

	font_atlas = custom_target(
		'font-atlas.tga',
		input: '../assets/envypn7x15.bdf',
		output: ['font_atlas.tga', 'font_atlas.h'],
		command: [fpack, '-o@OUTPUT0@', '-r@OUTPUT1@', '@INPUT@'],
		install: false,
		install_dir: [asset_install_path, false],
		build_by_default: true
	)
	flags += '-DINCLUDE_FONT_ATLAS'
	src += font_atlas[1]

	embedded_assets += [
		meson.source_root() + '/client/ui/opengl/shaders/chunks.vert',
		meson.source_root() + '/client/ui/opengl/shaders/ents.vert',
		meson.source_root() + '/client/ui/opengl/shaders/selection.vert',
		meson.source_root() + '/client/ui/opengl/shaders/text.vert',
		meson.source_root() + '/client/ui/opengl/shaders/text.frag',
		meson.source_root() + '/client/ui/opengl/shaders/world.frag',
		meson.source_root() + '/assets/obj/cube.obj',
		font_atlas,
	]
endif

embedded_assets += [
	meson.source_root() + '/assets/cfg/keymap.ini',
	meson.source_root() + '/assets/cfg/graphics.ini',
]

# Embedded assets
if embedded_assets.length() > 0
	embedded_data = custom_target(
		'embedded_data.h',
		input: embedded_assets,
		output: 'embedded_data.h',
		capture: true,
		command: [embed_binary] + embedded_assets,
		build_by_default: true
	)

	flags += '-DINCLUDE_EMBEDDED_DATA'
	src += embedded_data
endif

ini_h_config = [
	'-D_INI_CONFIGURED',
	'-DINI_ALLOW_MULTILINE=0',
	'-DINI_ALLOW_BOM=0',
	'-DINI_ALLOW_INLINE_COMMENTS=1',
	'-DINI_INLINE_COMMENT_PREFIXES=";#"',
	'-DINI_START_COMMENT_PREFIXES=";#"',
	'-DINI_ALLOW_NO_VALUE=0',
	'-DINI_STOP_ON_FIRST_ERROR=1',
	'-DINI_HANDLER_LINENO=1',
	'-DINI_CALL_HANDLER_ON_NEW_SECTION=0',
	'-DINI_USE_STACK=0',
	'-DINI_MAX_LINE=256',
	'-DINI_ALLOW_REALLOC=0',
	'-DINI_INITIAL_ALLOC=256'
]

crts_flags = flags + ini_h_config

executable(
	'crts',
	include_directories: shared_inc,
	sources: src,
	dependencies: deps,
	install: true,
	c_args: crts_flags,
	link_args: crts_flags
)
