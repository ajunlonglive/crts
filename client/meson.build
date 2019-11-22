src = files([
	'src/main.c',
	'src/net.c',
	'src/window.c',
	'../shared/geom.c',
	'../shared/update.c',
	'../shared/serialize.c',
	'../shared/alignment.c',
	'../shared/world.c',
	'../shared/queue.c'
])

deps = [
	cc.find_library('ncurses', required : true),
	cc.find_library('tinfo', required : true),
	cc.find_library('pthread', required : true),
]

executable('crtc', include_directories: shared_inc, sources: src, dependencies: deps, install: true)
