local ecs = require "ecs"

--main.c always calls this
ecs.progress_cb(function () end)

local array = ecs.array("ARR", "int32_t", 6)
ecs.set(ecs.Singleton, array, { 6, 5, 4, 2, 1 })

--This works
local strc = ecs.struct("STRC", "{int32_t x;int64_t w;}")
ecs.set(ecs.Singleton, strc, {x = 2, w = 400})
local f = ecs.get(ecs.Singleton, strc)
assert(f.x == 2)
assert(f.w == 400)