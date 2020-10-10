local ecs = require "ecs"

ecs.progress_cb(function() ecs.log("progress()!") end)

ecs.set_target_fps(60)

local wi = ecs.world_info()
ecs.log("target fps: " .. wi.target_fps, "last component: " .. wi.last_component_id)


local Position = ecs.struct("Position", "{float x; float y;}")
local Velocity = ecs.struct("Velocity", "{float x; float y;}")
local ents = ecs.bulk_new(10)

for i, e in ipairs(ents) do
    ecs.set(e, Position, { x = i * 10, y = i * 11})
    ecs.set(e, Velocity, { x = i * 12, y = i * 13})
end

local sys_id = 0

local function sys(it)
    local p, v = ecs.columns(it)

    assert(p == ecs.column(it, 1))
    assert(v == ecs.column(it, 2))

    for i = 1, it.count do
        assert(p[i].x == i * 10)
        assert(p[i].y == i * 11)

        assert(v[i].x == i * 12)
        assert(v[i].y == i * 13)
    end

    assert(it.system == sys_id)
    assert(not pcall(function () ecs.column(it, 3) end))
    assert(not pcall(function () ecs.columns(p) end))
end

sys_id = ecs.system(sys, "SYS", ecs.OnUpdate, "Position, Velocity")
