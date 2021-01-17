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

local sys_id = 0

local function sys(it)
    local count = 0
    for p, v, e in ecs.each(it) do
        count = count + 1
        --u.print_r(p)
        assert(p.x == count * 10)
        assert(p.y == count * 11)
        assert(v.x == count * 12)
        assert(v.y == count * 13)

        p.x = p.x + 1
        p.y = p.y + 2
        v.x = v.x + 3
        v.y = v.y + 4
    end

    --verify, restore values
    count = 0
    for p, v, e in ecs.each(it) do
        count = count + 1
        --u.print_r(p)
        assert(p.x == count * 10 + 1)
        assert(p.y == count * 11 + 2)
        assert(v.x == count * 12 + 3)
        assert(v.y == count * 13 + 4)

        p.x = p.x - 1
        p.y = p.y - 2
        v.x = v.x - 3
        v.y = v.y - 4
    end

    local p, v = ecs.columns(it)

    assert(ecs.is_owned(it, 1))
    assert(ecs.is_owned(it, 2))
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
    local p, v, s, e = ecs.columns(it)

    inc = inc + 1

    for i = 1, it.count do
        assert(p[i].x == i * 10)
        assert(p[i].y == i * 11)

        assert(v[i].x == i * 12)
        assert(v[i].y == i * 13)

        assert(s[i].v == inc - 1)
        s[i].v = inc

        assert(it.entities[i] > 0)
        assert(it.entities[i] == e[i])

        --Position is read-only, these have no effect
        p[i].x = p[i].x + 1
        p[i].y = p[i].y + 1
    end
end

ecs.system(sys_readonly, "sys_readonly", ecs.OnUpdate, "[in] Position, Velocity, LuaStruct")

local custom_context = false

local function sys_empty(it)
    local e = ecs.columns(it)

    if it.param ~= nil then
        assert(it.param[1] == "foo")

        if(it.param[3] == true) then
            it.interrupted_by = 400
        end

        if(it.param[2] == "custom_context") then
            custom_context = true
        end
    end

    assert(not pcall(function () return it.entities[1] > 0 end))

    for i = 1, it.count do
        assert(it.entities[i] > 0)
        assert(it.entities[i] == e[i])
    end
end

local em = ecs.system(sys_empty, "sys_empty", ecs.OnUpdate)

ecs.set_system_context(em, { "foo", "custom_context" })

ecs.run(em, 1.0)
assert(custom_context == true)

--ecs.run() temporarily overrides it.param
assert(ecs.run(em, 1.0) == 0)
assert(ecs.run(em, 1.0, {"foo", "bar"}) == 0)
assert(ecs.run(em, 1.0, {"foo", "bar", true}) == 400)

--make sure it.param is still set
custom_context = false
ecs.run(em, 1.0)
assert(custom_context == true)

--unset
custom_context = false
ecs.set_system_context(em, nil)
ecs.run(em, 1.0)
assert(custom_context == false)

local function trigger(it)
    local s = ecs.column(it, 1)

    for i=1, it.count do
        local e = it.entities[i]
        print("LuaStruct added for entity: " .. e)
    end
end

ecs.trigger(trigger, "OnAdd", ecs.OnAdd, "LuaStruct")

assert(not pcall(function () ecs.trigger(trigger, "name", ecs.OnAdd) end))
assert(not pcall(function () ecs.trigger(trigger, "name", ecs.OnUpdate, "LuaStruct") end))

ecs.add(ecs.new(), Struct)
ecs.add(ecs.new(), Struct)
ecs.add(ecs.new(), Struct)