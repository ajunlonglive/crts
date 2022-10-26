fpack = executable(
    'fpack',
    sources: files('fpack.c'),
    include_directories: include,
    build_by_default: false,
)

embed_binary = executable(
    'embed_binary',
    sources: files('embed_binary.c'),
    include_directories: include,
    build_by_default: false,
)

export_manifest = find_program('scripts/export_manifest')
download = find_program('scripts/download')
zip = find_program('scripts/zip')
