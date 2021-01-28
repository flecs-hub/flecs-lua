local t = require "test"
local ecs = require "ecs"
local u = require "util"

ecs.progress_cb(function() ecs.log("progress()!") end)


local HealthBuff = ecs.struct("HealthBuff", "{float x;}")
local ExpiryTimer = ecs.struct("ExpiryTimer", "{float expiry_time; float t;}");

local e = ecs.new()

ecs.add(e, HealthBuff)
--ecs.set(e, HealthBuff, { x = 10 })
ecs.set_trait(e, HealthBuff, ExpiryTimer, { expiry_time = 5, t = 0 })

local exp = ecs.get_trait(e, HealthBuff, ExpiryTimer)
assert(exp)
print("expiry_time: " .. exp.expiry_time)
assert(exp.expiry_time == 5.0)

local removed = false
local iter_count = 0

local function ExpireComponents(it)
    local et = ecs.column(it, 1)

    local trait = ecs.column_entity(it, 1)
    local comp = trait & ((1 << 31) - 1)

    assert(ecs.get_typeid(trait) == ExpiryTimer)
    assert(comp == HealthBuff)

    iter_count = iter_count + 1

    for i = 1, it.count do

        print("t = " .. et[i].t .. ", delta: " .. it.delta_time)
        print("expiry: " .. et[i].expiry_time)

        et[i].t = et[i].t + it.delta_time;

        if(et[i].t >= et[i].expiry_time) then

            assert(removed == false)
            removed = true

            print("removing...")

            ecs.remove(it.entities[i], comp)
            ecs.remove(it.entities[i], trait)
        end
    end
end

ecs.system(ExpireComponents, "ExpireComponents", ecs.OnUpdate, "TRAIT | ExpiryTimer")

ecs.progress(1)
ecs.progress(1)
ecs.progress(1)
ecs.progress(1)
ecs.progress(1)
ecs.progress(1)


assert(not ecs.has(e, HealthBuff))
assert(not ecs.has(e, ExpiryTimer))

assert(removed == true)
print("iter_count: " .. iter_count)