project(
	'crts',
	'c',
	version: '0.1.0',
	license: 'GPL-3.0-only',
	meson_version: '>=0.48.0',
	default_options: [
		'c_std=c99',
		'warning_level=2',
		'default_library=static'
	],
)

cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
	'-O2',
	'-Wundef',
	'-Wlogical-op',
	'-Wmissing-include-dirs',
	'-Wold-style-definition',
	'-Wpointer-arith',
	'-Winit-self',
	'-Wstrict-prototypes',
	'-Wimplicit-fallthrough=2',
	'-Wendif-labels',
	'-Wstrict-aliasing=2',
	'-Woverflow',

	'-Wno-missing-braces',
	'-Wno-missing-field-initializers',
	'-Wno-unused-parameter',
	'-flto=thin'
]), language: 'c')
add_global_link_arguments(
	'-flto=thin', language: 'c'
)

shared_inc = include_directories(['shared'])

subdir('client')
subdir('server')
subdir('test')
