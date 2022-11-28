flags = global_flags
foreach target : get_option('targets')
    flags += '-DCRTS_HAVE_' + target
endforeach

src = files(
    'assets.c',
    'launcher.c',
    'opts.c',
    'save_settings.c',
)

if need_ui
    if 'opengl' in get_option('uis')
        src += files('ui.c')
    endif
endif

# Embedded assets
if get_option('embed_assets')
    src += custom_target(
        'embedded_data.h',
        input: embedded_assets_static + embedded_assets_gen,
        output: 'embedded_data.h',
        capture: true,
        command: [embed_binary] + embedded_assets_static + embedded_assets_gen_path,
    )
    flags += '-DINCLUDE_EMBEDDED_DATA'
else
    assets_static += embedded_assets_static
    assets_gen += embedded_assets_gen
    assets_gen_path += embedded_assets_gen_path
endif

# Export manifest
src += custom_target(
    'asset manifest',
    output: 'asset_manifest.h',
    capture: true,
    command: [export_manifest] + assets_static + assets_gen_path + embedded_assets_static +
    embedded_assets_gen_path,
)

crts_exe = executable(
    'crts',
    c_args: flags,
    dependencies: crts_dependencies,
    include_directories: include,
    install: get_option('packaging') == 'unix',
    link_with: crts_link_with,
    sources: src,
)
