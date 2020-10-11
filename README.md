# flecs-lua

This is a Lua script host library for [Flecs](https://github.com/SanderMertens/flecs).

## [Lua API](ecs.lua)


### Dependencies

* Flecs v2.2+ with all modules and addons enabled

* [flecs-meta](https://github.com/flecs-hub/flecs-meta)

* Lua 5.3 or later

## Build

### Meson

Meson 0.55.0 or later is required

```bash
meson build # -Dtests=enabled
cd build
ninja
ninja test
```
