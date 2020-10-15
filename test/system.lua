local ecs = require "ecs"
local u = require "util"

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

local q = ecs.query_new("Position, Velocity")
local it = ecs.query_iter(q)
local q_count = 0

while ecs.query_next(it) do
    local p, v = ecs.columns(it)
    --u.print_r(it.columns)
    q_count = q_count + 1

    for i = 1, it.count do
        print("p[i].x = " .. p[i].x)
        assert(p[i].x == i * 10)
    end
end

print("count :" .. q_count)
assert(q_count == 1)

local sys_id = 0

local function sys(it)
    local p, v = ecs.columns(it)

    assert(#it.columns == 2)
    assert(p == it.columns[1])
    assert(v == it.columns[2])
    assert(p == ecs.column(it, 1))
    assert(v == ecs.column(it, 2))

    assert(it.system == sys_id)
    assert(not pcall(function () ecs.column(it, 3) end))
    assert(not pcall(function () ecs.columns(p) end))

    for i = 1, it.count do
        assert(p[i].x == i * 10)
        assert(p[i].y == i * 11)

        assert(v[i].x == i * 12)
        assert(v[i].y == i * 13)
    end

end

sys_id = ecs.system(sys, "SYS", ecs.OnUpdate, "Position, Velocity")

local Struct = ecs.struct("LuaStruct", "{int32_t v;}")

local inc = 1

for i, e in ipairs(ents) do
    ecs.set(e, Struct, { v = inc})
end

local function sys_readonly(it)
    local p, v, s = ecs.columns(it)

    inc = inc + 1

    for i = 1, it.count do
        assert(p[i].x == i * 10)
        assert(p[i].y == i * 11)

        assert(v[i].x == i * 12)
        assert(v[i].y == i * 13)

        assert(s[i].v == inc - 1)
        s[i].v = inc

        --Position is read-only, these have no effect
        p[i].x = p[i].x + 1
        p[i].y = p[i].y + 1
    end
end

ecs.system(sys_readonly, "sys_readonly", ecs.OnUpdate, "[in] Position, Velocity, LuaStruct")
