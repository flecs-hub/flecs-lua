local t = require "test"
local ecs = require "ecs"
local u = require "util"

u.test_defaults()


local system_a_invoked = false
local system_b_invoked = false
local system_c_invoked = false


local function SystemA(it)
    ecs.log("time: ", it.delta_time)
    --assert(it.delta_time == 1.0)
    --assert(it.delta_system_time == 3.0)
    system_a_invoked = true
end

local function SystemB(it)
    assert(it.delta_time == 1)
    assert(it.delta_system_time == 3)
    system_b_invoked = true
end

local function SystemC(it)
    assert(it.delta_time == 1.0)
    assert(it.delta_system_time == 6.0)
    system_b_invoked = true
end

local Position = ecs.struct("Position", "{float x; float y;}")
ecs.new(nil, "Position")

local system_a = ecs.system(SystemA, "SystemA", ecs.OnUpdate, Position)

local filter = ecs.set_rate_filter(system_a, 3, 0)
assert(filter ~= 0)
assert(filter == system_a)


assert(system_a_invoked == false)
ecs.progress(1.0)
assert(system_a_invoked == false)
ecs.progress(1.0)
assert(system_a_invoked == false)

ecs.progress(1.0)
assert(system_a_invoked == true)

system_a_invoked = false

ecs.progress(1.0)
assert(system_a_invoked == false)
ecs.progress(1.0)
assert(system_a_invoked == false)
ecs.progress(1.0)
assert(system_a_invoked == true)

