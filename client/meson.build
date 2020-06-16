src = files([
	'../lib/inih/ini.c',
	'../shared/constants/blueprints.c',
	'../shared/constants/globals.c',
	'../shared/math/geom.c',
	'../shared/math/rand.c',
	'../shared/messaging/client_message.c',
	'../shared/net/ack.c',
	'../shared/net/bind_sock.c',
	'../shared/net/connection.c',
	'../shared/net/defs.c',
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
	'../shared/util/log.c',
	'../shared/util/mem.c',
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

flags = []

if get_option('ncurses-ui')
	src += files([
		'ui/ncurses/container.c',
		'ui/ncurses/graphics.c',
		'ui/ncurses/info.c',
		'ui/ncurses/ui.c',
		'ui/ncurses/window.c',
		'ui/ncurses/world.c',
	])

	flags += '-DNCURSES_UI'

	ncursesw = dependency('ncursesw', required: false,
		static: get_option('static'))
	if ncursesw.found()
			deps += [ncursesw]
	else
			deps += [dependency('ncurses', required: true,
				static: get_option('static'))]
	endif
endif

if get_option('opengl-ui')
	src += files([
		'../lib/glad/src/gl.c',
		'../shared/math/linalg.c',
		'../shared/util/text.c',
		'ui/opengl/color_cfg.c',
		'ui/opengl/globals.c',
		'ui/opengl/input.c',
		'ui/opengl/obj_loader.c',
		'ui/opengl/render.c',
		'ui/opengl/render/chunks.c',
		'ui/opengl/render/ents.c',
		'ui/opengl/render/hud.c',
		'ui/opengl/render/selection.c',
		'ui/opengl/render/text.c',
		'ui/opengl/tgaloader.c',
		'ui/opengl/ui.c',
		'ui/opengl/winutil.c',
	])

	flags += '-DOPENGL_UI'

	libglfw = dependency('glfw3', required: false)
	if not libglfw.found()
		libglfw = dependency('glfw', required: true)
	endif

	deps += [
		libm,
		#PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig
		# TODO: get these to link statically
		libglfw
	]

	if build_macos_app
		asset_path = '.'
		installed_asset_path = 'Contents/Resources'
		install_subdir('../assets', install_dir: 'Contents/Resources')
	else
		asset_path = meson.current_build_dir()
		installed_asset_path = asset_path
	endif

	font_atlas = custom_target(
		'font-atlas.tga',
		input: '../assets/envypn7x15.bdf',
		output: ['font_atlas.tga', 'font_atlas.h'],
		command: [fpack, '-o@OUTPUT0@', '-r@OUTPUT1@', '@INPUT@'],
		install: true,
		install_dir: installed_asset_path,
		build_by_default: true
	)

	flags += '-DINCLUDE_FONT_ATLAS'
	src += font_atlas[1]

	flags += '-DCRTS_ASSET_PATH="' + asset_path + '"'

	if get_option('opengl-ui-wayland')
		deps += dependency('wayland-client', required: true)
	endif
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
