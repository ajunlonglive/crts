cereal_src = files([
	'src/cereal.c',
	'../shared/messaging/server_message.c',
	'../shared/messaging/client_message.c',
	'../shared/serialize/base.c',
	'../shared/serialize/geom.c',
	'../shared/serialize/server_message.c',
	'../shared/serialize/client_message.c',
	'../shared/sim/ent.c',
	'../shared/sim/action.c',
	'../shared/constants/action_info.c',
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
	'../shared/constants/tile_chars.c',
	'../shared/util/mem.c',
])

terra_deps = [
	cc.find_library('m', required : true),
]

executable('terra', include_directories: shared_inc, dependencies: terra_deps, sources: terra_src, install: false)

hashy_src = files([
	'src/hashy.c',
	'../shared/types/hash.c',
])

executable('hashy', include_directories: shared_inc, sources: hashy_src, install: false)

path_src = files([
	'src/path.c',
	'../server/src/sim/terrain.c',
	'../shared/types/hash.c',
	'../shared/sim/chunk.c',
	'../shared/sim/world.c',
	'../shared/math/geom.c',
	'../shared/math/perlin.c',
	'../shared/util/mem.c',
	'../shared/constants/tile_chars.c',
	'../server/src/sim/pathfind/heap.c',
	'../server/src/sim/pathfind/meander.c',
	'../server/src/sim/pathfind/mapping.c',
	'../server/src/sim/pathfind/pathfind.c',
	'../server/src/sim/pathfind/pgraph.c',
	'../server/src/sim/pathfind/vector_field.c',
	'../server/src/sim/pathfind/vector_field.h',
])

path_deps = [
	cc.find_library('m', required : true),
]

executable('path', include_directories: shared_inc, sources: path_src, dependencies: path_deps, install: false)
