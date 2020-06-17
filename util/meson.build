fpack_src = files([
	'fpack.c'
])

fpack = executable('fpack', sources: fpack_src, dependencies: [], install: false)

embed_binary_src = files([
	'embed_binary.c'
])

embed_binary = executable(
	'embed_binary',
	sources: embed_binary_src,
	include_directories: shared_inc,
	dependencies: [],
	install: false)
