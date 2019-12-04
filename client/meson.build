src = files([
	'src/main.c',
	'src/net.c',
	'src/window.c',
	'src/drawers.c',
	'src/state.c',
])

deps = [
	cc.find_library('ncurses', required : true),
	cc.find_library('tinfo', required : true),
	cc.find_library('pthread', required : true),
]

executable('crtc', include_directories: shared_inc, sources: src, dependencies: deps, install: true)
