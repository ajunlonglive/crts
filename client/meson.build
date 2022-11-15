src = files(
    'ui/common.c',
    'client.c',
    'cmdline.c',
    'handle_msg.c',
    'input_cfg.c',
    'input_handler.c',
    'opts.c',
)

flags = global_flags
dependencies = crts_dependencies

# UIs

if 'curses' in get_option('uis')
    src += files(
        'ui/term/graphics.c',
        'ui/term/graphics_cfg.c',
        'ui/term/info.c',
        'ui/term/ui.c',
        'ui/term/world.c',
    )
endif

if 'opengl' in get_option('uis')
    src += files(
        'ui/gl/cmdline.c',
        'ui/gl/colors.c',
        'ui/gl/globals.c',
        'ui/gl/input.c',
        'ui/gl/render.c',
        'ui/gl/render/chunks.c',
        'ui/gl/render/ents.c',
        'ui/gl/render/final_quad.c',
        'ui/gl/render/hud.c',
        'ui/gl/render/selection.c',
        'ui/gl/render/settings_menu.c',
        'ui/gl/render/shadows.c',
        'ui/gl/render/sun.c',
        'ui/gl/render/water.c',
        'ui/gl/shader.c',
        'ui/gl/shader_multi_obj.c',
        'ui/gl/ui.c',
    )

    if get_option('debug')
        src += files('ui/gl/render/pathfinding_overlay.c')
    endif

    dependencies += [libglfw]
endif

crts_link_with += library(
    'crts_client',
    c_args: flags,
    dependencies: dependencies,
    include_directories: include,
    sources: src,
)
