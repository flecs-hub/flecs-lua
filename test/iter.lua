local t = require "test"
local ecs = require "ecs"
local u = require "util"

u.test_defaults()

local AppContext = ecs.struct("AppContext", "{int32_t whatever;}")
local Position = ecs.struct("Position", "{float x; float y;}")
local Velocity = ecs.struct("Velocity", "{float x; float y;}")

ecs.bulk_new(Position, 15)
ecs.bulk_new(Position, 20)

ecs.new(0, nil, "Position")
ecs.set(0, Position, { 100, 200 })

---@type ecs_iter_t
local it = ecs.term_iter(Position)

ecs.term_next(it)

print("entity count: " .. it.count)
print("term count: " .. #it.columns)

assert(it.count > 10)


assert(not pcall(function () ecs.term_iter() end))
assert(not pcall(function () ecs.term_iter(0) end))


it = ecs.filter_iter({ terms = Position })

assert(ecs.filter_next(it))
print("entity count: " .. it.count)
assert(it.count > 10)


it = ecs.filter_iter({ terms = { ecs.MetaType, ecs.MetaTypeSerializer } })

local count = 0

while ecs.filter_next(it) do
    count = count + it.count
end

print("total count: " .. count)

assert(count > 10)

--[[ XXX: this should work on v3
local ent = ecs.new("ent", "Velocity")

ecs.set(ent, Velocity, { 321, 555 })

q = ecs.query("ent:Velocity")
it = ecs.query_iter(q)

assert(it.count == 1)

local shared = it.columns[1]

assert(shared.x == 321)
assert(shared.y == 555)

assert(ecs.has(ent, Velocity))

print("COUNT: ", it.count)
--assert(it.count == 1)
]]--