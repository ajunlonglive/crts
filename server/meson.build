src = files([
	'aggregate_msgs.c',
	'handle_msg.c',
	'main.c',
	'net.c',
	'opts.c',
	'sim/action.c',
	'sim/do_action.c',
	'sim/do_action/build.c',
	'sim/do_action/carry.c',
	'sim/do_action/fight.c',
	'sim/do_action/harvest.c',
	'sim/do_action/move.c',
	'sim/ent.c',
	'sim/ent_lookup.c',
	'sim/environment.c',
	'sim/sim.c',
	'sim/storehouse.c',
	'sim/update_tile.c',
	'sim/worker.c',
])

include = [shared_inc]
c_args = []
cpp_args = []

if get_option('enable-tracy')
	src += files(get_option('tracy-dir') + '/TracyClient.cpp')
	c_args += ['-DTRACY_ENABLE']
	cpp_args += ['-DTRACY_ENABLE']
	include += include_directories([get_option('tracy-dir')])
endif

executable(
	'server',
	c_args: c_args,
	cpp_args: cpp_args,
	dependencies: [libm],
	include_directories: include,
	install: true,
	install_dir: get_option('subcmd-dir'),
	link_with: [libcrts_shared, libterragen],
	sources: src,
)
