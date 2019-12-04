cereal_src = files([
	'src/cereal.c',
	'../shared/messaging/server_message.c',
	'../shared/serialize/base.c',
	'../shared/serialize/geom.c',
	'../shared/serialize/server_message.c',
	'../shared/sim/ent.c',
	'../shared/sim/alignment.c',
])

executable('cereal', include_directories: shared_inc, sources: cereal_src, install: false)

terra_src = files([
	'src/terra.c',
	'../server/src/sim/terrain.c',
	'../shared/types/hash.c',
	'../shared/sim/chunk.c',
	'../shared/sim/world.c',
	'../shared/math/geom.c',
	'../shared/math/perlin.c',
])

deps = [
	cc.find_library('m', required : true),
]

executable('terra', include_directories: shared_inc, dependencies: deps, sources: terra_src, install: false)

hashy_src = files([
	'src/hashy.c',
	'../shared/types/hash.c',
])

executable('hashy', include_directories: shared_inc, sources: hashy_src, install: false)
