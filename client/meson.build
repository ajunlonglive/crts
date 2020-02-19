src = files([
	'../lib/inih/ini.c',
	'../shared/constants/globals.c',
	'../shared/math/geom.c',
	'../shared/messaging/client_message.c',
	'../shared/serialize/base.c',
	'../shared/serialize/client_message.c',
	'../shared/serialize/geom.c',
	'../shared/serialize/server_message.c',
	'../shared/sim/action.c',
	'../shared/sim/alignment.c',
	'../shared/sim/chunk.c',
	'../shared/sim/ent.c',
	'../shared/sim/world.c',
	'../shared/types/hash.c',
	'../shared/types/queue.c',
	'../shared/util/mem.c',
	'../shared/util/time.c',
	'cfg/keymap.c',
	'display/container.c',
	'display/painters.c',
	'display/window.c',
	'draw.c',
	'graphics.c',
	'hiface.c',
	'input/action_handler.c',
	'input/handler.c',
	'input/move_handler.c',
	'main.c',
	'net/receive.c',
	'net/respond.c',
	'net/server_cx.c',
	'request_missing_chunks.c',
	'world_update.c',
])

deps = [
	cc.find_library('ncurses', required : true),
]

ini_h_config = '-D_INI_CONFIGURED -DINI_ALLOW_MULTILINE=0 -DINI_ALLOW_BOM=0 -DINI_ALLOW_INLINE_COMMENTS=1 -DINI_INLINE_COMMENT_PREFIXES=";#" -DINI_START_COMMENT_PREFIXES=";#" -DINI_ALLOW_NO_VALUE=0 -DINI_STOP_ON_FIRST_ERROR=1 -DINI_HANDLER_LINENO=1 -DINI_CALL_HANDLER_ON_NEW_SECTION=0 -DINI_USE_STACK=0 -DINI_MAX_LINE=256 -DINI_ALLOW_REALLOC=0 -DINI_INITIAL_ALLOC=256'
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
