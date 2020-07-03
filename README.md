# crts

A real-time-simulation game written in c.

[website](https://mochiro.moe/crts)

# build

## dependencies

+ `glfw3` if you want the opengl ui
  - `wayland-client` if you are using wayland
+ `ncurses` if you want the ncurses ui

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
