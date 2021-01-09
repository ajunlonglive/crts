foreach f: [
	'ci',
	'gallery.html',
	'index.html',
	'style.css',
]
	configure_file(
		configuration: version_info,
		input: f,
		output: f,
		install_dir: get_option('website_dest')
	)
endforeach
