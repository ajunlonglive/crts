make_icns = find_program('./make_icns')
	'icon.icns',
	input: meson.source_root() + '/assets/icon.png',
	output: 'icon.icns',
	command: [make_icns, '@OUTDIR@', '@INPUT@', '@OUTPUT@'],
	build_by_default: true,
	install_dir: asset_install_path
)

install_data('launcher', install_dir: 'Contents/MacOS')
install_data('Info.plist', install_dir: 'Contents')
