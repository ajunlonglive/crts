# crts

A real-time-simulation game written in c.

[website](https://mochiro.moe/crts)

# build

## dependencies

+ [meson](https://mesonbuild.com/Getting-meson.html) / [ninja](https://ninja-build.org/)
+ [glfw3](https://www.glfw.org/) if you want the opengl ui
  - [wayland-client](https://wayland.freedesktop.org/) if you are using wayland
+ [ncurses](https://invisible-island.net/ncurses/) if you want the ncurses ui

## compile

```
$ meson build
$ ninja -C build
```

# usage

First start the server, then start the client[s].

## server

```
$ bin/s -h
```

## client

```
$ bin/c -h
```
