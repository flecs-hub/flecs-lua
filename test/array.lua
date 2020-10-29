local ecs = require "ecs"

--main.c always calls this
ecs.progress_cb(function () end)

local MetaType = ecs.lookup_fullpath("flecs.meta.MetaType")

--This works
local strc = ecs.struct("STRC", "{int32_t x;int64_t w;}")
ecs.set(ecs.Singleton, strc, {x = 2, w = 400})
local f = ecs.get(ecs.Singleton, strc)
assert(f.x == 2)
assert(f.w == 400)

local meta = ecs.get(strc, MetaType)
print("size: ".. meta.size)
assert(meta.size == 16)

local array = ecs.array("ARR", "int32_t", 6)
ecs.set(ecs.Singleton, array, { 6, 5, 4, 2, 1 })


meta = ecs.get(array, MetaType)

assert(meta.size == 4 * 6)
assert(meta.alignment == 4)


local array2 = ecs.array("ARR2", "STRC", 6)

meta = ecs.get(array2, MetaType)

print("size: ".. meta.size)
assert(meta.size == 16 * 6)
assert(meta.alignment == 8)