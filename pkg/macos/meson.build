makeicns = find_program('../../bin/makeicns')
custom_target(
	'icon.icns',
	input: '../../assets/icon.png',
	output: 'icon.icns',
	command: [makeicns, meson.current_build_dir(), '@INPUT@', '@OUTPUT@'],
	install: true,
	build_by_default: true,
	install_dir: 'Contents/Resources'
)

install_subdir('../../client/ui/opengl/shaders',
	install_dir: 'Contents/Resources/client/ui/opengl')
install_subdir('../../cfg',
	install_dir: 'Contents/Resources')
install_data('launcher', install_dir: 'Contents/MacOS')
install_data('Info.plist', install_dir: 'Contents')

install_data(font_atlas[0].full_path(), install_dir: 'Contents/Resources')

meson.add_install_script('../../bin/bundleglfw')
