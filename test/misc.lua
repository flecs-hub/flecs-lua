local ecs = require "ecs"

local runs = 0

ecs.progress_cb(function()
    ecs.log("progress()!")

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
print("elapsed: " .. ecs.time_measure(time) .. "seconds")

time.nanosec = 2 << 40
assert(not pcall(function () ecs.time_measure(time) end))

time.sec = 2 << 34
time.nanosec = 0
assert(not pcall(function () ecs.time_measure(time) end))

ecs.tracing_enable(3)

runs = 2

while runs > 0 and ecs.progress(0)  do
    runs = runs - 1
end

local wi = ecs.world_info()
assert(wi.frame_count_total == 2)