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
add_project_arguments(
    cc.get_supported_arguments(
        [
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
        ],
    ),
    language: 'c',
)

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

if host_machine.system() == 'windows'
    platform = 'windows'
else
    platform = 'posix'
endif

include = [include_directories(['include'])]
global_flags = ['-DCRTS_COMPTIME', '-DCRTS_PLATFORM_' + platform]

assets_static = []
assets_gen = []
assets_gen_path = []
embedded_assets_static = []
embedded_assets_gen = []
embedded_assets_gen_path = []

crts_dependencies = []
crts_link_with = []

dist_file = false

# dependencies

need_ui = false
foreach tgt : ['client', 'terragen']
    if tgt in get_option('targets')
        need_ui = true
    endif
endforeach

if need_ui
    if 'curses' in get_option('uis')
        crts_dependencies += dependency('curses', static: get_option('static'))
        global_flags += '-DNCURSES_UI'

    endif

    if 'opengl' in get_option('uis')
        glfw3 = subproject('glfw3', default_options: ['warning_level=1'])
        libglfw = glfw3.get_variable('glfw3_dep')
        global_flags += '-DOPENGL_UI'
    endif

    if get_option('sound')
        soundio = subproject(
            'libsoundio',
            default_options: ['default_library=static'],
        )
        libsoundio = soundio.get_variable('libsoundio_dep')
        global_flags += ['-DHAVE_SOUND', '-DSOUNDIO_STATIC_LIBRARY']
    endif
endif

# tracy

tracy_dep = dependency(
    'tracy',
    required: get_option('tracy'),
    default_options: ['cpp_std=c++11'],
)
if tracy_dep.found()
    add_languages('cpp', native: false)
    global_flags += ['-DTRACY_ENABLE']
    crts_dependencies += tracy_dep
endif

# targets

subdir('build_util')
subdir('assets')
subdir('shared')

foreach target : ['tests', 'server', 'client', 'terragen', 'website']
    if target in get_option('targets')
        subdir(target)
    endif
endforeach

subdir('launcher')

# packaging

subdir('build_util/pkg')

if get_option('upload') and dist_file != false
    meson.add_install_script('build_util/scripts/upload', dist_file.full_path())
endif
