# flecs-lua

This is a Lua script host library for [Flecs](https://github.com/SanderMertens/flecs).

### Dependencies

* Flecs v2.2+ with all modules and addons enabled

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

```c
/* Creates a script host for the world */
ECS_IMPORT(world, FlecsLua);

/* Get a pointer to the VM */
const EcsLuaHost *host = ecs_singleton_get(world, EcsLuaHost);
lua_State *L = host->L;

/* Execute init script, the world for all API calls is implicit */
luaL_dofile(L, argv[1]);

/* Lua systems will run on the main thread */
while(ecs_progress(world, 0))

```

The script executed on init should be similar to the program's `main()`.

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
    local p = ecs.column(it, 1)

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

