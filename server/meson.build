src = files(
    'sim/ai.c',
    'sim/ent.c',
    'sim/environment.c',
    'sim/sim.c',
    'sim/update_tile.c',
    'aggregate_msgs.c',
    'handle_msg.c',
    'opts.c',
    'server.c',
    #'sim/ent_lookup.c',
)

crts_link_with += library(
    'crts_server',
    c_args: global_flags,
    dependencies: crts_dependencies,
    include_directories: include,
    sources: src,
)
