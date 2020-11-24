local ecs = require "ecs"
local u = require "util"

ecs.progress_cb(function() ecs.log("progress()!") end)

local Position = ecs.struct("Position", "{float x; float y;}")
local Velocity = ecs.struct("Velocity", "{float x; float y;}")
local ents = ecs.bulk_new(10)

for i, e in ipairs(ents) do
    ecs.set(e, Position, { x = i * 10, y = i * 11})
    ecs.set(e, Velocity, { x = i * 12, y = i * 13})
end


local q = ecs.query("Position, Velocity")
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

ecs.subquery(q, "Position")

it = ecs.query_iter(q)

while ecs.query_next(it) do
    local p = ecs.columns(it)
end

assert(it.count == 10)