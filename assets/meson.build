if get_option('packaging') == 'unix'
	crts_sharedir = get_option('prefix') + '/' + get_option('datadir') + '/crts'
	asset_path = crts_sharedir + '/assets'
	asset_install_path = asset_path
elif get_option('packaging') == 'windows'
	asset_path = './resources'
	asset_install_path = asset_path
else
	asset_path = '.'
	asset_install_path = asset_path
endif

global_flags += '-DCRTS_ASSET_PATH="' + asset_path + '"'

# asset bundle
subdir('bundle_manifest')

fetchbundle=false
foreach asset : bundled_assets
	r = run_command(file_exists, meson.current_source_dir(), 'bundle/' + asset)
	if r.returncode() != 0
		message('asset "' + asset + '" not found, asset bundle will be downloaded')
		fetchbundle=true
		break
	endif
endforeach

if fetchbundle
	asset_bundle = custom_target(
		'assets bundle',
		output: bundled_assets,
		command: [
			download,
			'https://mochiro.moe/crts/dist/assets.zip',
			meson.current_build_dir()
		],
		build_by_default: true,
	)

	assets_gen += asset_bundle.to_list()
	foreach asset : asset_bundle.to_list()
		assets_gen_path += asset.full_path()
	endforeach
else
	foreach asset : bundled_assets
		assets_static += files(['bundle/' + asset])
	endforeach
endif

if need_ui
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
			install: true,
			install_dir: [asset_install_path, false],
			build_by_default: true
		)

		assets_gen += font_atlas[0]
		assets_gen_path += font_atlas[0].full_path()

		subdir('obj')
		subdir('shaders')

		assets_static += files(['cfg/opengl.ini', 'water.tga'])
	endif
endif
