embedded_assets_static += files(
    'basic.frag',
    'chunk_bottoms.vert',
    'chunks.vert',
    'chunks_depth.vert',
    'empty.frag',
    'final_quad.frag',
    'final_quad.vert',
    'instanced_model_depth.vert',
    'points.vert',
    'selection.vert',
    'shapes.frag',
    'shapes.vert',
    'sun.frag',
    'sun.vert',
    'terragen_mesh.frag',
    'terragen_mesh.vert',
    'terragen_pixels.frag',
    'terragen_pixels.vert',
    'text.frag',
    'text.vert',
    'water.frag',
    'water.vert',
)

normal_interpolation_flat = configuration_data(
    {'normal_interpolation': 'flat'},
)

normal_interpolation_smooth = configuration_data(
    {'normal_interpolation': 'smooth'},
)

foreach shader : [
    'world.in.frag',
    'instanced_model.in.vert',
]
    parts = shader.split('.')
    base = parts[0]
    ext = parts[-1]

    generated = [
        configure_file(
            input: shader,
            output: base + '_flat.' + ext,
            configuration: normal_interpolation_flat,
        ),
        configure_file(
            input: shader,
            output: base + '_smooth.' + ext,
            configuration: normal_interpolation_smooth,
        ),
    ]

    foreach g : generated
        assets_gen += g
        assets_gen_path += meson.current_source_dir() / '@0@'.format(g)
    endforeach
endforeach
