
src = files([
	'src/cfg/keymap.c',
	'src/display.c',
	'src/display/container.c',
	'src/display/painters.c',
	'src/display/window.c',
	'src/input/handler.c',
	'src/main.c',
	'src/net/receive.c',
	'src/net/respond.c',
	'src/net/server_cx.c',
	'src/world_update.c',

	'../lib/inih/ini.c',

	'../shared/math/geom.c',
	'../shared/types/queue.c',
	'../shared/types/hash.c',
	'../shared/serialize/base.c',
	'../shared/serialize/geom.c',
	'../shared/serialize/server_message.c',
	'../shared/serialize/client_message.c',
	'../shared/messaging/client_message.c',
	'../shared/sim/world.c',
	'../shared/sim/ent.c',
	'../shared/sim/alignment.c',
])
add_project_arguments([
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
	'-DINI_INITIAL_ALLOC=256',
], language: 'c')


deps = [
	cc.find_library('ncurses', required : true),
	cc.find_library('tinfo', required : true),
	cc.find_library('pthread', required : true),
]

executable('crtc', include_directories: shared_inc, sources: src, dependencies: deps, install: true)
