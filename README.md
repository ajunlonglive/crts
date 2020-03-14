# crts

A real-time-simulation [game] written in c (crts).

For a quick overview, you can [watch a short
demo](https://asciinema.org/a/310102) on asciinema.

# installing

First make sure the required git submodules are checked out:

```
$ git submodule init
$ git submodule update
```

Next configure the build directory with meson:

```
$ meson build --buildtype release
```

Finally build all targets with ninja:

```
$ ninja build
```

If you followed the above instructions, the server will be located at
`build/server/crtsd` and the client will be located at `build/client/crts`.

# usage

First start the server, then start the client[s].

## server

```
$ crtsd <SEED>

SEED: an integer
```

## client

```
$ crts [OPTIONS]

OPTIONS:
-i <id> : set client_id (default: random())
-s <ip> : set the server ip (default: 127.0.0.1)
```

# interacting

Interaction is done through keyboard maps, similar to vim maps (leader keys
rather than key combinations).  The default key map is located at
`cfg/keymap.ini`.  Also, input may be preceded by a series of digits [0-9] that
may affect the action.  For instance the `cursor_*` commands will parse the
typed integer and move that many characters, rather than 1.
