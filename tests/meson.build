tests = [
    {'src': 'path.c'},
    {'src': 'snap.c', 'manual': true},
    {'src': 'path_local.c'},
    {'src': 'hdarr.c'},
    {'src': 'hash.c'},
    {'src': 'readlines.c', 'args': [files('readlines.c')]},
    {'src': 'inih.c'},
    {'src': 'shrink.c'},
    {'src': 'save.c'},
    {'src': 'random.c', 'manual': true},
    {'src': 'blur.c', 'manual': true},
    {'src': 'heap.c'},
    {'src': 'sack.c'},
    {'src': 'message_sack.c'},
    {'src': 'rudp.c'},
]

foreach t : tests
    src = t['src']
    name = src.split('.')[0]
    args = []

    if t.has_key('args')
        args = t['args']
    endif

    test_exe = executable(
        name,
        dependencies: [],
        include_directories: include,
        link_with: [libcrts_shared],
        sources: [src],
    )

    if not (t.has_key('manual') and t['manual'])
        test(name, test_exe, args: args)
    endif
endforeach
