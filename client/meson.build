src = files([
	'assets.c',
	'cfg/graphics.c',
	'cfg/keymap.c',
	'handle_msg.c',
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
])

deps = []
link_with = [libcrts_shared, libcrts_server, libterragen]

flags = global_flags


# debug extras
if get_option('debug')
	src += files(['input/debug.c'])
endif

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

if get_option('opengl-ui')
	src += files([
		'cfg/opengl.c',
		'ui/opengl/cmdline.c',
		'ui/opengl/globals.c',
		'ui/opengl/input.c',
		'ui/opengl/keymap_hook.c',
		'ui/opengl/loaders/color_cfg.c',
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

	flags += '-DOPENGL_UI'
	link_with += libcrts_gl_shared
	deps += gl_shared_deps
endif

if client_assets['static'].length() > 0
	# Embedded assets
	if get_option('embed-assets')
		src += custom_target(
			'embedded_data.h',
			input: client_assets['static'] + client_assets['gen'],
			output: 'embedded_data.h',
			capture: true,
			command: [embed_binary] + client_assets['static'] + client_assets['gen_path'],
			build_by_default: true
		)
		flags += '-DINCLUDE_EMBEDDED_DATA'
	else
		assets_to_install += client_assets['static']
	endif

	# Export manifest
	src += custom_target(
		'asset manifest',
		output: 'asset_manifest.h',
		capture: true,
		command: [export_manifest] + client_assets['static'] + client_assets['gen_path'],
		build_by_default: true
	)
endif

executable(
	'client',
	c_args: flags,
	dependencies: deps,
	include_directories: shared_inc,
	install: true,
	install_dir: get_option('subcmd-dir'),
	link_with: link_with,
	sources: src
)
