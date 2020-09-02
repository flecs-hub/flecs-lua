# flecs-lua

This is a Lua script host library for Flecs.

## [Lua API](api.md)

## Build

### Meson

Meson 0.55.0 or later is required but 0.55.1 may fail to build with MSVC,
in that case install the previous version:
```c
pip install 'meson==0.55.0' #--force-reinstall
```

```bash
meson build # -Dtests=enabled
cd build
ninja
ninja test
```
