local t = require "test"
local ecs = require "ecs"
local u = require "util"

local runs = 0

ecs.progress_cb(function(delta_time)

    ecs.log("progress()!")

    assert(delta_time == 2.0)

    runs = runs + 1
    assert(runs <= 1)

    ecs.quit()
end)

ecs.log("This is a LOG message: ", 1234, ecs.OnStore)
ecs.err("This is an ERROR message")
ecs.dbg("This is a DEBUG message")
ecs.warn("This is a WARN message")

ecs.assert(1)
ecs.assert(1, "test")
assert(not pcall(ecs.assert(0)))
assert(not pcall(function () ecs.assert(false, "failing assert") end))

local time = ecs.get_time()

print("time = " .. time.sec .. " seconds, " .. time.nanosec .. " nanoseconds")
print("elapsed: " .. ecs.time_measure(time) .. " seconds")
print("time__tostring: ", time)

time.nanosec = 2 << 40
assert(not pcall(function () ecs.time_measure(time) end))

time.sec = 2 << 34
time.nanosec = 0
assert(not pcall(function () ecs.time_measure(time) end))

ecs.tracing_enable(3)
ecs.tracing_enable(0)

runs = 2

while runs > 0 and ecs.progress(0)  do
    runs = runs - 1
end

local wi = ecs.world_info()
assert(wi.frame_count_total == 2)

ecs.createtable()

ecs.createtable(10)

ecs.createtable(10, 10)

local Struct = ecs.struct("Struct", "{int32_t x; int32_t y; int32_t z;}")
local Enum = ecs.enum("Enum", "{ Minus = -1, Zero, One, Two, Three }")
local Bitmask = ecs.bitmask("Bitmask", "{ One = 1, Two = 2, Four = 4, Seven = 7, SixtyFour = 64 }" )

ecs.zero_init(Struct)

local e = ecs.new()

local zero = ecs.get_mut(e, Struct)

assert(zero.x == 0)
assert(zero.y == 0)
assert(zero.z == 0)

local ret = ecs.meta_constants(Enum)

assert(ret.Minus == -1)
assert(ret.Three == 3)

assert(ecs.meta_constants(Bitmask, ret) == ret)

assert(ret.Two == 2)
assert(ret.Seven == 7)
assert(ret.SixtyFour == 64)

assert(not pcall(function () ecs.meta_constants(Struct) end))
assert(not pcall(function () ecs.meta_constants(ecs.Timer) end))

--local ws = ecs.world_stats()
--assert(ws.entity_count ~= 0)

--local e = ecs.lookup_fullpath("flecs.lua.LuaWorldStats")
--print(ecs.emmy_class(e))


local w2 = ecs.init()

w2.delete(w2.bulk_new(100))

--w2.struct("Test", "{int32_t x;}")

ecs.new(4444, "world_test")
w2.new(4444, "world_test2")

assert(ecs.name(4444) == "world_test")
assert(w2.name(4444) == "world_test2")

w2.fini()
assert(not pcall(function () w2.fini() end))
assert(not pcall(function () ecs.fini() end))

local w3 = ecs.init()
--w3.struct("Test", "{int32_t x;}")
--w3.query("Test")
--Let garbage collection take care of it

ecs.progress_cb = function () end

--require "entity"
