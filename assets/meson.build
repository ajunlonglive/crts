# Asset path

if get_option('packaging') == 'macos'
	asset_path = '.'
	asset_install_path = 'Contents/Resources'
else
	crts_sharedir = get_option('prefix') + '/' + get_option('datadir') + '/crts'
	asset_path =  crts_sharedir + '/assets'
	asset_install_path = asset_path
endif

global_flags += '-DCRTS_ASSET_PATH="' + asset_path + '"'

assets_static += files(['cfg/keymap.ini', 'cfg/graphics.ini'])

if 'opengl' in get_option('uis')
	font_atlas = custom_target(
		'font_atlas.tga',
		input: '../assets/envypn7x15.bdf',
		output: ['font_atlas.tga', 'font_atlas.h'],
		command: [fpack, '-o@OUTPUT0@', '-r@OUTPUT1@', '@INPUT@'],
		install: not get_option('embed_assets'),
		install_dir: [asset_install_path, false],
		build_by_default: true
	)

	assets_gen += font_atlas[0]
	assets_gen_path += font_atlas[0].full_path()

	subdir('obj')
	subdir('shaders')

	assets_static += files(['cfg/opengl.ini', 'water.tga'])
endif

export_manifest = find_program(meson.source_root() + '/build_util/scripts/export_manifest')
