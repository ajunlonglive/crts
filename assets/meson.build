# Asset path
crts_sharedir = get_option('prefix') + '/' + get_option('datadir') + '/crts'
asset_path =  crts_sharedir + '/assets'
asset_install_path = asset_path

if get_option('build-macos-app')
	asset_path = '.'
	asset_install_path = 'Contents/Resources'
endif

global_flags += '-DCRTS_ASSET_PATH="' + asset_path + '"'

client_assets += { 'static': client_assets['static'] + files(['cfg/keymap.ini', 'cfg/graphics.ini']) }

if libglfw.found()
	font_atlas = custom_target(
		'font_atlas.tga',
		input: '../assets/envypn7x15.bdf',
		output: ['font_atlas.tga', 'font_atlas.h'],
		command: [fpack, '-o@OUTPUT0@', '-r@OUTPUT1@', '@INPUT@'],
		install: not get_option('embed-assets'),
		install_dir: [asset_install_path, false],
		build_by_default: true
	)

	client_assets += { 'gen': client_assets['gen'] + [font_atlas[0]] }
	terragen_assets += { 'gen': terragen_assets['gen'] + [font_atlas[0]] }
	client_assets += { 'gen_path': client_assets['gen_path'] + [font_atlas[0].full_path()] }
	terragen_assets += { 'gen_path': terragen_assets['gen_path'] + [font_atlas[0].full_path()] }

	subdir('obj')
	subdir('shaders')

	client_assets += { 'static': client_assets['static'] + files(['cfg/opengl.ini', 'water.tga']) }
endif

export_manifest = find_program(meson.source_root() + '/build_util/scripts/export_manifest')
