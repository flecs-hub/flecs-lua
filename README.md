# flecs.lua

This is a Lua script hosting library for Flecs.

## Lua API

#### `ecs.new([id])`

Create a new entity with a unique id or `id` if given.

Returns the entity id.

#### `ecs.delete(id)`

Delete an entity and all of its components.

#### `ecs.name(id)`

Get the name of an entity.

This will return the name as specified in the `EcsName` component.

#### `ecs.lookup(name)`

Look an entity by name


## Build

### Meson

Meson 0.55.0 or later is required.

```bash
meson build # -Dtests=enabled
ninja -C build
```