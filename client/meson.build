src = files([
	'cfg/keymap.c',
	'client.c',
	'handle_msg.c',
	'input/action_handler.c',
	'input/cmdline.c',
	'input/handler.c',
	'input/helpers.c',
	'input/keymap.c',
	'input/move_handler.c',
	'opts.c',
	'request_missing_chunks.c',
	'ui/common.c',
])

if get_option('debug')
	src += files(['input/debug.c'])
endif

flags = global_flags
dependencies = []

# UIs

if 'curses' in get_option('uis')
	src += files([
		'ui/ncurses/container.c',
		'ui/ncurses/graphics.c',
		'ui/ncurses/graphics_cfg.c',
		'ui/ncurses/info.c',
		'ui/ncurses/ui.c',
		'ui/ncurses/window.c',
		'ui/ncurses/world.c',
	])
endif

if 'opengl' in get_option('uis')
	src += files([
		'cfg/opengl.c',
		'ui/opengl/cmdline.c',
		'ui/opengl/colors.c',
		'ui/opengl/globals.c',
		'ui/opengl/input.c',
		'ui/opengl/keymap_hook.c',
		'ui/opengl/render.c',
		'ui/opengl/render/chunks.c',
		'ui/opengl/render/ents.c',
		'ui/opengl/render/hud.c',
		'ui/opengl/render/selection.c',
		'ui/opengl/render/shadows.c',
		'ui/opengl/render/sun.c',
		'ui/opengl/render/water.c',
		'ui/opengl/shader.c',
		'ui/opengl/shader_multi_obj.c',
		'ui/opengl/ui.c',
	])

	if get_option('debug')
		src += files(['ui/opengl/render/pathfinding_overlay.c'])
	endif

	dependencies += [libglfw]
endif

crts_link_with += library(
	'crts_client',
	c_args: flags,
	dependencies: dependencies,
	include_directories: include,
	sources: src
)
