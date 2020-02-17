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

host_os = host_machine.system()

cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
	'-DNDEBUG', # for heap library
	'-DHASH_STATS',
	'-Wundef',
	'-Wlogical-op',
	'-Wpadding',
	'-Wmissing-include-dirs',
	'-Wold-style-definition',
	'-Wpointer-arith',
	'-Winit-self',
	'-Wstrict-prototypes',
	'-Wimplicit-fallthrough=2',
	'-Wendif-labels',
	'-Wstrict-aliasing=2',
	'-Woverflow',

	'-fstrict-aliasing',

	'-Wno-missing-braces',
	'-Wno-missing-field-initializers',
	'-Wno-unused-parameter',
	'-flto=thin'
]), language: 'c')

shared_inc = include_directories(['include'])

subdir('client')
subdir('server')
subdir('test')
