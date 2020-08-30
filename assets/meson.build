# Asset path

crts_sharedir = get_option('prefix') + '/' + get_option('datadir') + '/crts'
asset_path =  crts_sharedir + '/assets'
asset_install_path = asset_path

if get_option('build-macos-app')
	asset_path = '.'
	asset_install_path = 'Contents/Resources'
endif

global_flags += '-DCRTS_ASSET_PATH="' + asset_path + '"'

# font atlas
font_atlas = custom_target(
	'font-atlas.tga',
	input: '../assets/envypn7x15.bdf',
	output: ['font_atlas.tga', 'font_atlas.h'],
	command: [fpack, '-o@OUTPUT0@', '-r@OUTPUT1@', '@INPUT@'],
	install: false,
	install_dir: [asset_install_path, false],
	build_by_default: true
)

subdir('obj')
subdir('shaders')
