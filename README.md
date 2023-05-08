# flecs-lua

This is a Lua binding for [Flecs](https://github.com/SanderMertens/flecs),
it can be used to extend an application or as a standalone module.

### Dependencies

* Flecs v3.2.0 with most addons enabled - backward and forward compatibility is limited due to dependencies on semi-private API's

* Lua 5.3 or later (requires 64-bit integers)

## Build

### Meson >= 0.55.0

```bash
meson build # -Dtests=enabled
cd build
ninja
ninja test
```

## [Lua API](ecs.lua)

## Usage

Scripts are hosted by the `FlecsLua` module for the world it's imported into.

Using this library as a standalone Lua module is possible but is not the main focus of the project.

```c
/* Creates a script host for the world */
ECS_IMPORT(world, FlecsLua);

/* Get a pointer to the VM */
lua_State *L = ecs_lua_get_state(world);

/* Execute init script, the world for all API calls is implicit */
luaL_dofile(L, argv[1]);

/* Lua systems will run on the main thread */
while(ecs_progress(world, 0))
```

Most of the functions are bound to their native counterparts,
components are (de-)serialized to- and from Lua, it is not designed
to be used with LuaJIT where FFI is preferred.

Components defined from the host with the `meta` module and from Lua are
handled the same way and can be mixed in systems, queries, etc.

The script executed on init should be similar to the host's `main()`.

#### **main.lua**

```lua
local ecs = require "ecs"
local m = require "module"

local ents = ecs.bulk_new(m.Position, 10)

for i in 1, #ents do
    ecs.set(ents[i], m.Position, { x = i * 2, y = i * 3})
end
```

Modules can be stuctured like normal Lua modules.

Importing native modules or other Lua scripts is left to the host application.
`ecs.import()` does not load flecs modules, instead it returns a table with
all components and named entities from an already imported flecs module.

Flecs modules are defined and imported with `ecs.module()`,
it takes an import callback which is called immediately and returns the imported module ID.
All components and entities should be registered inside the callback function for proper scoping.

#### **module.lua**

```lua
local ecs = require "ecs"
local m = {}

function m.system(it)

    for p, e in ecs.each(it) do
        print("entity: " .. e)
        p.x = p.x + 1
        p.y = p.y + 1
    end
end

ecs.module("name", function()

    m.Position = ecs.struct("Position", "{float x; float y;}")

    ecs.system(m.system, "Move", ecs.OnUpdate, "Position")

end)

return m
```

### Debugging

For debug builds (`#ifndef NDEBUG`) most API functions will retrieve the current file,
source line and expose them through `ar.short_src` and `ar.currentline`.
Set watches for these variables to track where API functions are called from in Lua.

### Lua state lifecycle

* A default Lua state is created when the module is imported

* To set a custom `lua_State` use `ecs_lua_set_state()`, this will destroy the default state.

* In both cases `lua_close()` is called on `ecs_fini()`
to trigger `__gc` metamethods before the world is destroyed.

