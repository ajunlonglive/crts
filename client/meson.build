src = files([
	'../shared/constants/globals.c',
	'../shared/math/geom.c',
	'../shared/math/rand.c',
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
	'../shared/serialize/chunk.c',
	'../shared/serialize/coder.c',
	'../shared/serialize/message.c',
	'../shared/serialize/net.c',
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
]) + [
	configure_file(configuration: version_info,
		input: files('../include/version.h'), output: 'version.h')
]

deps = []

assets = [
	meson.source_root() + '/assets/cfg/keymap.ini',
	meson.source_root() + '/assets/cfg/graphics.ini',
]

flags = global_flags

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
	src += gl_src + files([
		'../lib/glad/src/gl.c',
		'../shared/math/linalg.c',
		'../shared/opengl/loaders/obj.c',
		'../shared/opengl/loaders/shader.c',
		'../shared/opengl/loaders/tga.c',
		'../shared/opengl/render/text.c',
		'../shared/opengl/shader.c',
		'../shared/opengl/util.c',
		'../shared/opengl/window.c',
		'../shared/util/file_formats/tga.c',
		'cfg/opengl.c',
		'ui/opengl/cmdline.c',
		'ui/opengl/globals.c',
		'ui/opengl/input.c',
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

	flags += gl_flags

	deps += gl_deps

	assets += [
		meson.source_root() + '/assets/cfg/opengl.ini',
		meson.source_root() + '/assets/water.tga',
	]
	assets += client_shaders
	assets += gl_assets
	assets += obj_assets
endif

if assets.length() > 0
	# Embedded assets
	if get_option('embed-assets')
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
	else
		assets_to_install += assets
	endif

	# Export manifest
	export_manifest = find_program(meson.source_root() + '/bin/export_manifest')
	asset_manifest = custom_target(
		'asset manifest',
		output: 'asset_manifest.h',
		capture: true,
		command: [export_manifest] + managed_assets + assets,
		build_by_default: true
	)
	src += asset_manifest
	flags += '-DINCLUDE_EXPORTED_MANIFEST'
endif

executable(
	'client',
	c_args: flags,
	dependencies: deps,
	include_directories: shared_inc,
	install: true,
	install_dir: crts_libexecdir,
	link_args: flags,
	sources: src
)
