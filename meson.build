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

if get_option('enable-tracy')
	add_languages('cpp', native: false)
endif

# version information

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

# global variables

shared_inc = include_directories(['include'])
global_flags = ['-DCRTS_COMPTIME', '-DCRTS_PLATFORM_POSIX']

assets_to_install = []
client_assets = { 'static': [], 'gen': [], 'gen_path': [] }
terragen_assets = { 'static': [], 'gen': [], 'gen_path': [] }

subcmds = []

# dependencies

libm = cc.find_library('m', required : true, static: get_option('static'))

if get_option('opengl-ui')
	glfw3 = subproject('glfw3', default_options: ['warning_level=1'])
	libglfw = glfw3.get_variable('glfw3_dep')
endif

libncurses = dependency('curses', required: get_option('ncurses-ui'),
	static: get_option('static'))

libthreads = dependency('threads', required: get_option('build-terragen'))

# targets

optional_targets = [
	{'name': 'server',   'subcmd': true }, # TODO: must be before client
	{'name': 'client',   'subcmd': true },
	{'name': 'terragen', 'subcmd': true },
	{'name': 'tests',    'subcmd': false},
]

subdir('build_util')
subdir('assets')
subdir('shared')
subdir('terragen/gen') # optional?

foreach t: optional_targets
	if get_option('build-' + t['name'])
		if t['subcmd']
			subcmds += t['name']
		endif
		subdir(t['name'])
	endif
endforeach

subdir('util')

# packaging

if get_option('build-macos-app')
	subdir('build_util/pkg/macos')
endif

if assets_to_install.length() > 0
	install_data(assets_to_install, install_dir: asset_install_path)
endif
