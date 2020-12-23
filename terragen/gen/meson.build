terragen_srcs = files([
	'erosion.c',
	'faults.c',
	'filters.c',
	'gen.c',
	'gen.c',
	'opts.c',
	'rasterize.c',
	'write_tiles.c',
])

libterragen = library(
	'terragen',
	dependencies: [libm],
	include_directories: shared_inc,
	install: false,
	sources: terragen_srcs,
)
