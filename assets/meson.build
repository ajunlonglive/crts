install_assets = get_option('embed_assets') == false

if get_option('packaging') == 'unix'
	crts_sharedir = get_option('prefix') + '/' + get_option('datadir') + '/crts'
	asset_path = crts_sharedir + '/assets'
	asset_install_path = asset_path
else
	asset_path = '.'
	asset_install_path = asset_path
endif

global_flags += '-DCRTS_ASSET_PATH="' + asset_path + '"'

assets_static += files(['cfg/keymap.ini'])

if 'curses' in get_option('uis')
	assets_static += files(['cfg/curses.ini'])
endif

if 'opengl' in get_option('uis')
	font_atlas = custom_target(
		'font_atlas.tga',
		input: 'envypn7x15.bdf',
		output: ['font_atlas.tga', 'font_atlas.h'],
		command: [fpack, '-o@OUTPUT0@', '-r@OUTPUT1@', '@INPUT@'],
		install: install_assets,
		install_dir: [asset_install_path, false],
		build_by_default: true
	)

	assets_gen += font_atlas[0]
	assets_gen_path += font_atlas[0].full_path()

	subdir('obj')
	subdir('shaders')

	assets_static += files(['cfg/opengl.ini', 'water.tga'])
endif
