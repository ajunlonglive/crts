obj_raw = [
#         source	      epsilon
	['cube.obj',         '0'      ],
	['deer.obj',         '0.0001' ],
	['dodecahedron.obj', '0'      ],
	['tree.obj',         '0.0'    ],
	['pyramid.obj',      '0'      ],
]

obj_reduce = find_program(meson.source_root() + '/bin/obj-reduce')

obj_assets = []

foreach obj : obj_raw
	obj_assets += custom_target(
		obj[0],
		input: obj[0],
		output: obj[0],
		command: [obj_reduce, '@INPUT@', '@OUTPUT@', obj[1]],
		build_by_default: true
	)
endforeach
