src = files([
	'erosion.c',
	'faults.c',
	'filters.c',
	'gen.c',
	'gen.c',
	'opts.c',
	'rasterize.c',
	'write_tiles.c',
])

crts_link_with += library(
	'terragen',
	dependencies: [libm],
	include_directories: include,
	sources: src,
)
