cereal_src = files([
	'src/cereal.c',
	'../shared/update.c',
	'../shared/serialize.c',
	'../shared/alignment.c',
	'../shared/world.c',
])

executable('cereal', include_directories: shared_inc, sources: cereal_src, install: false)

terra_src = files([
	'src/terra.c',
	'../server/src/terrain.c',
	'../shared/world.c',
	'../shared/chunk.c',
	'../shared/geom.c',
	'../shared/hash.c',
	'../shared/perlin.c',
])

deps = [
	cc.find_library('m', required : true),
]

executable('terra', include_directories: shared_inc, dependencies: deps, sources: terra_src, install: false)

hashy_src = files([
	'src/hashy.c',
	'../shared/geom.c',
	'../shared/hash.c',
])

executable('hashy', include_directories: shared_inc, sources: hashy_src, install: false)
