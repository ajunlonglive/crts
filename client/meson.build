src = files([
	'src/main.c',
	'src/net.c',
	'src/window.c',
])

deps = [
	cc.find_library('ncurses', required : true),
	cc.find_library('tinfo', required : true)
]

executable('crtc', include_directories: shared_inc, sources: src, dependencies: deps, install: true)
