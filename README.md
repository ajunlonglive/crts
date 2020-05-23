# crts

[![builds.sr.ht status](https://builds.sr.ht/~lattis/crts.svg)](https://builds.sr.ht/~lattis/crts?)

A real-time-simulation game written in c.

# build

requirements:

+ meson
+ ninja
+ ncurses, with headers i.e. ncurses-dev
+ a c compiler

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
```

## client

```
$ crts [OPTIONS]
```
