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

# compiler setup

cc = meson.get_compiler('c')
add_project_arguments(cc.get_supported_arguments([
	'-Wno-missing-braces',
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

# version information

git = find_program('git', required: false)
if git.found()
	git_rev_parse = run_command(git, 'rev-parse', '--short', '@')
	git_sha = git_rev_parse.stdout().strip()
else
	git_sha = 'unknown'
endif

version_info = {
	'version': meson.project_version(),
	'vcs_tag': git_sha,
}

# global variables

include = [include_directories(['include'])]
global_flags = [
	'-DCRTS_COMPTIME',
	'-DCRTS_PLATFORM_POSIX'
]

assets_static = []
assets_gen = []
assets_gen_path = []

crts_dependencies = []
crts_link_with = []

dist_file = false

# dependencies

libm = cc.find_library('m', required : true, static: get_option('static'))

if 'curses' in get_option('uis')
	crts_dependencies += dependency('curses', static: get_option('static'))
	global_flags += '-DNCURSES_UI'

endif

if 'opengl' in get_option('uis')
	glfw3 = subproject('glfw3', default_options: ['warning_level=1'])
	libglfw = glfw3.get_variable('glfw3_dep')
	crts_dependencies += libglfw
	global_flags += '-DOPENGL_UI'
endif

# tracy

if get_option('tracy')
	add_languages('cpp', native: false)
	global_flags += ['-DTRACY_ENABLE']
	include += include_directories([get_option('tracy_dir')])
endif

# targets

subdir('build_util')
subdir('assets')
subdir('shared')

foreach target: get_option('targets')
	subdir(target)
endforeach

subdir('launcher')

# packaging

subdir('build_util/pkg/' + get_option('packaging'))

if get_option('upload') and dist_file
	meson.add_install_script('build_util/scripts/upload', dist_file.full_path())
endif
