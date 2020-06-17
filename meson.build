project(
	'crts',
	'c',
	version: '0.1.1',
	license: 'GPL-3.0-only',
	meson_version: '>=0.54.0',
	default_options: [
		'c_std=c11',
		'warning_level=3',
		'default_library=static',
		'b_ndebug=if-release',
	],
)

git = find_program('git', required: false)

if git.found()
	git_rev_parse = run_command(git, 'rev-parse', '--short', '@')
	git_sha = git_rev_parse.stdout().strip()
else
	git_sha = 'dist'
endif

version_info = {
	'version': meson.project_version(),
	'vcs_tag': git_sha,
}

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
global_flags = []

build_macos_app = get_option('build-macos-app')

# Asset path

asset_path = '/usr/local/share/crts/assets'
asset_install_path = asset_path

if build_macos_app
	asset_path = '.'
	asset_install_path = 'Contents/Resources'
endif

global_flags += '-DCRTS_ASSET_PATH="' + asset_path + '"'

subdir('util')
subdir('client')
subdir('server')
subdir('test')

if build_macos_app
	subdir('pkg/macos')
endif
