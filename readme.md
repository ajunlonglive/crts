# crts

A real time simulation written from scratch in c

[website](https://mochiro.moe/crts)

# build

## dependencies

In order to build crts you need
[meson](https://mesonbuild.com/Getting-meson.html), and a supported
[backend](https://mesonbuild.com/Running-Meson.html).  You will also need a C
toolchain.

Optional dependencies:

+ curses
+ [glfw3 compile
  dependencies](https://www.glfw.org/docs/latest/compile_guide.html#compile_deps)
  for your preferred backend

A patched glfw is used that includes a meson build, so cmake is *not* required.

*Note*: Currently building on windows is only supported under
[cygwin](https://www.cygwin.com/), but native support is planned.

## compile

```
meson setup build .
meson compile -C build
```

Build options can be displayed using `meson configure build`.

## install

```
meson install -C build
```

# usage

See `crts -h`.
