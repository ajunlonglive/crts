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

global_flags = []

libm = cc.find_library('m', required : true, static: get_option('static'))
libglfw = dependency('glfw3', required: get_option('opengl-ui'),
	static: get_option('static'))
libncurses = dependency('ncurses', required: get_option('ncurses-ui'),
	static: get_option('static'))

assets_to_install = []
client_assets = { 'static': [], 'gen': [], 'gen_path': [] }
terragen_assets = { 'static': [], 'gen': [], 'gen_path': [] }

subcmds = []

if get_option('enable-tracy')
	add_languages('cpp')
endif

subdir('build_util')

subdir('assets')

# common opengl config
if libglfw.found()
	gl_deps = [libm, libglfw]

	libwayland_client = dependency('wayland-client',
		required: get_option('opengl-ui-wayland'),
		static: get_option('static'))

	if libwayland_client.found()
		gl_deps += libwayland_client
	endif

	gl_src = [font_atlas[1]] + files([
		'lib/glad/src/gl.c',
		'shared/opengl/loaders/shader.c',
		'shared/opengl/loaders/tga.c',
		'shared/opengl/render/text.c',
		'shared/opengl/shader.c',
		'shared/opengl/util.c',
		'shared/opengl/window.c',
		'shared/util/file_formats/tga.c',
	])
	gl_flags = ['-DOPENGL_UI', '-DINCLUDE_FONT_ATLAS']
endif

if get_option('build-client')
	subcmds += 'client'
	subdir('client')
endif

if get_option('build-terragen')
	subcmds += 'terragen'
	subdir('terragen')
endif

if get_option('build-server')
	subcmds += 'server'
	subdir('server')
endif

subdir('util')

if get_option('build-tests')
	subdir('test')
endif

if get_option('build-macos-app')
	subdir('pkg/macos')
endif

if assets_to_install.length() > 0
	install_data(assets_to_install, install_dir: asset_install_path)
endif
