# flecs.lua

This is a Lua script host library for Flecs.

## Lua API

#### `ecs.new([[[id : 0], name : string], components : string])`

Create a new entity with `id` or a unique id,
if `name` is given a named entity will be created.
The entity can be optionally initialized with a 
set of `components`.

Returns the entity id.

#### `ecs.delete(id)`

Delete an entity and all of its components.

#### `ecs.name(id)`

Get the name of an entity.

This will return the name as specified in the `EcsName` component.

#### `ecs.lookup(name)`

Look up an entity by name

#### `ecs.array(name, desc)`

Create an array component.

format for `desc`: `(type,X)`

Returns the component entity

#### `ecs.struct(name, desc)`

Create a struct component.

format for `desc`: `{type member; ...}`

Returns the component entity


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
