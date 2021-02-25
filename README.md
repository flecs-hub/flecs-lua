# flecs-lua

This is a Lua script host library for [Flecs](https://github.com/SanderMertens/flecs).

### Dependencies

* Flecs v2.3+ with all modules and addons enabled

* [flecs-meta](https://github.com/flecs-hub/flecs-meta)

* Lua 5.3 or later

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

The ability to 'require "ecs"' as a standalone Lua module is currently not maintained.

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

Components defined from the host with flecs-meta and from Lua are
handled the same way and can be mixed in systems, queries, etc.

The script executed on init should be similar to the host's `main()`.

**main.lua**

```lua
local ecs = require "ecs"
local m = require "module"

local ents = ecs.bulk_new(m.Position, 10)

for i in 1, #ents do
    ecs.set(ents[i], m.Position, { x = i * 2, y = i * 3})
end
```

Modules can be stuctured like normal Lua modules.

Flecs modules are defined with `ecs.module()`, note that they are imported automatically,
all components and entities should be registered inside the callback function
for proper scoping.

**module.lua**

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

### Lua state lifecycle

* A default Lua state is created when the module is imported

* To set a custom state use `ecs_lua_set_state()`

* In both cases `lua_close()` is called on `ecs_fini()`
to trigger `__gc` metamethods before the world is destroyed.

