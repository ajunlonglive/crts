project(
	'crts',
	'c',
	version: '0.1.0',
	license: 'GPL-3.0-only',
	meson_version: '>=0.51.0',
	default_options: [
		'c_std=c99',
		'warning_level=2',
		'default_library=static',
		'b_ndebug=if-release',
	],
)

host_os = host_machine.system()

cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
	'-Wendif-labels',
	'-Wimplicit-fallthrough=2',
	'-Winit-self',
	'-Wlogical-op',
	'-Wmissing-include-dirs',
	'-Wno-missing-braces',
	'-Wno-missing-field-initializers',
	'-Wno-unused-parameter',
	'-Wold-style-definition',
	'-Woverflow',
	'-Wpointer-arith',
	'-Wstrict-aliasing=2',
	'-Wstrict-prototypes',
	'-Wundef',
	'-fstrict-aliasing',
]), language: 'c')

shared_inc = include_directories(['include'])
libm = cc.find_library('m', required : true, static: get_option('static'))

if get_option('valgrind-backtrace')
	add_project_arguments(['-DVALGRIND_BACKTRACE'], language: 'c')
endif

subdir('client')
subdir('server')
subdir('test')
