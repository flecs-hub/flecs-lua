local t = require "test"
local ecs = require "ecs"
local u = require "util"

u.test_defaults()

local struct_str = "{float x; float y; float z;}"

local Position = ecs.struct("Position", struct_str)
local Velocity = ecs.struct("Velocity", struct_str)

local ExpiryTimer = ecs.struct("ExpiryTimer", "{float expiry_time; float t;}");

local removed = false
local iter_count = 0

local function ExpireComponents(it)
    local et = ecs.column(it, 1)

    local pair = ecs.column_entity(it, 1)
    local comp = ecs.pair_object(pair)

    assert(ecs.get_typeid(pair) == ExpiryTimer)

    iter_count = iter_count + 1

    for i = 1, it.count do

        print("t = " .. et[i].t .. ", delta: " .. it.delta_time)
        print("expiry: " .. et[i].expiry_time)

        et[i].t = et[i].t + it.delta_time;

        if(et[i].t >= et[i].expiry_time) then
            print("removing...")
            ecs.remove(it.entities[i], comp)
            ecs.remove(it.entities[i], pair)
        end
    end
end

ecs.system(ExpireComponents, "ExpireComponents", ecs.OnUpdate, { expr = "(ExpiryTimer, *)" })

local e = ecs.new()

ecs.add(e, Position)
ecs.add(e, Velocity)

ecs.set_pair(e, ExpiryTimer, Position, { expiry_time = 3, t = 0})
ecs.set_pair(e, ExpiryTimer, Velocity, { expiry_time = 2, t = 0})

assert(ecs.has_pair(e, ExpiryTimer, Position))


local et = ecs.get_pair(e, ExpiryTimer, Position)
assert(et)
u.asserteq(et.expiry_time, 3)

ecs.progress(1)

assert(ecs.has_pair(e, ExpiryTimer, Velocity))

ecs.progress(1)

assert(not ecs.has_pair(e, ExpiryTimer, Velocity))
assert(ecs.has_pair(e, ExpiryTimer, Position))


ecs.progress(1)

assert(not ecs.has_pair(e, ExpiryTimer, Position))

ecs.progress(1)
ecs.progress(1)
ecs.progress(1)

print("iter_count: " .. iter_count)