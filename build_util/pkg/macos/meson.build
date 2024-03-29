make_icns = find_program('./make_icns')
icon_icns = custom_target(
    'icon.icns',
    input: meson.source_root() + '/assets/icon.png',
    output: 'icon.icns',
    command: [make_icns, '@OUTDIR@', '@INPUT@', '@OUTPUT@'],
)

info_plist = configure_file(
    configuration: version_info,
    input: 'Info.plist',
    output: 'Info.plist',
)

make_app = find_program('./make_app')
app_name = 'crts.app'
app_binaries = [crts_exe] + files('launcher')
app_contents = [info_plist]
app_resources = [icon_icns]
app_resources_gen = app_resources

app_resources_gen += assets_static + assets_gen
app_resources += assets_static + assets_gen_path

app = custom_target(
    app_name,
    input: app_contents + app_binaries + app_resources_gen,
    output: app_name,
    command: [
        make_app,
        '@OUTDIR@/' + app_name,
        '-c', app_contents,
        '-b', app_binaries,
        '-r', app_resources,
    ],
    build_by_default: true,
)

if get_option('upload')
    dist_filename = app_name + '-' + version_info['version'] + '.zip'
    dist_file = custom_target(
        dist_filename,
        input: app,
        output: dist_filename,
        command: [zip, '@OUTDIR@/' + dist_filename, '@INPUT@'],
        build_by_default: true,
    )
endif
