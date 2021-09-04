local t = require "test"
local ecs = require "ecs"
local u = require "util"

u.test_defaults()

local ent = ecs.new()
local tag = ecs.new()

ecs.add(ent, tag)

local snapshot = ecs.snapshot()

local it = ecs.snapshot_iter(snapshot)

local b = false
local tcount = 0

while ecs.snapshot_next(it) do

    tcount = tcount + it.count
    assert(#it.columns == 0)

    for e in ecs.each(it) do
        --print("e:", e)
        if e == ent then
            b = true
        end
    end
end

--print("tcount:", tcount)
assert(b == true)

ecs.delete(ent)

assert(not ecs.is_alive(ent))

ecs.snapshot_restore(snapshot)

--Snapshots is now collected, it can't restored twice
assert(not pcall(function () ecs.snapshot_restore(snapshot) end))

assert(ecs.is_alive(ent))

snapshot = ecs.snapshot()

local w2 = ecs.init()

--Snapshots can only be restored to the same world
assert(not pcall(function () w2.snapshot_restore(snapshot) end))
