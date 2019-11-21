src = files([
	'src/main.c',
	'src/net.c',
	'src/sim.c',
	'../shared/world.c',
	'../shared/queue.c',
	'../shared/serialize.c',
	'../shared/alignment.c',
	'../shared/globals.c',
	'../shared/geom.c',
	'../shared/action.c'
])

deps = [
	cc.find_library('pthread', required : true),
]

executable('crtcd', include_directories: shared_inc, sources: src, dependencies: deps, install: true)
