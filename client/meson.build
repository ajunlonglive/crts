src = files([
	'../lib/inih/ini.c',
	'../shared/constants/blueprints.c',
	'../shared/constants/globals.c',
	'../shared/math/geom.c',
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
	'../shared/sim/alignment.c',
	'../shared/sim/chunk.c',
	'../shared/sim/ent.c',
	'../shared/sim/world.c',
	'../shared/types/darr.c',
	'../shared/types/hash.c',
	'../shared/types/hdarr.c',
	'../shared/util/log.c',
	'../shared/util/mem.c',
	'../shared/util/time.c',
	'cfg/cfg.c',
	'cfg/graphics.c',
	'cfg/keymap.c',
	'display/container.c',
	'display/info.c',
	'display/window.c',
	'display/world.c',
	'draw.c',
	'graphics.c',
	'hiface.c',
	'input/action_handler.c',
	'input/handler.c',
	'input/keymap.c',
	'input/move_handler.c',
	'main.c',
	'net.c',
	'opts.c',
	'request_missing_chunks.c',
	'world_update.c',
])

ncursesw = dependency('ncursesw', required: false, static: get_option('static'))
if ncursesw.found()
		deps = [ncursesw]
else
		deps = [dependency('ncurses', required: true)]
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

crts_flags = ini_h_config

executable(
	'crts',
	include_directories: shared_inc,
	sources: src,
	dependencies: deps,
	install: true,
	c_args: crts_flags,
	link_args: crts_flags
)
