makeicns = find_program('./makeicns')
custom_target(
	'icon.icns',
	input: meson.source_root() + '/assets/icon.png',
	output: 'icon.icns',
	command: [makeicns, meson.current_build_dir(), '@INPUT@', '@OUTPUT@'],
	install: true,
	build_by_default: true,
	install_dir: asset_install_path
)

install_data('launcher', install_dir: 'Contents/MacOS')
install_data('Info.plist', install_dir: 'Contents')
