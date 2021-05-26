# crts

A game and engine written from scratch in c

[website](https://mochiro.moe/crts)

# build

## dependencies

In order to build crts you need
[meson](https://mesonbuild.com/Getting-meson.html), and a supported
[backend](https://mesonbuild.com/Running-Meson.html).  You will also need a C
toolchain. Windows builds require the [mingw-w64](http://mingw-w64.org/)
toolchain.

Optional dependencies:

+ curses
+ [glfw3 compile
  dependencies](https://www.glfw.org/docs/latest/compile_guide.html#compile_deps)
  for your preferred backend
+ [libsoundio](http://libsound.io/) compile dependencies for your preferred
  backend(s)

A patched glfw/libsoundio is used that builds with meson, so cmake is *not*
required.

## compile

```
meson setup build .
meson compile -C build
```

Build options can be displayed using `meson configure build`.

Note: On linux, you need to manually specify your glfw backend (either `x11` or
`wayland`) like this:

```
meson setup -Dglfw3:glfw_backend=<your backend>
```

# usage

If you are on Linux/macOS, the simplest way to run it is `bin/crts`.  The main
binary is located at `build/launcher/crts`, and platform distribution packages
(e.g. `crts.app`) are located under `build/build_util/pkg/<platform>/`. See
`crts -h` for additional options.

For controls, please see `assets/cfg/keymap.ini`
