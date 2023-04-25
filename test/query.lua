local t = require "test"
local ecs = require "ecs"
local u = require "util"

u.test_defaults()

local AppContext = ecs.struct("AppContext", "{int32_t whatever;}")
local Position = ecs.struct("Position", "{float x; float y;}")
local Velocity = ecs.struct("Velocity", "{float x; float y;}")
local Foo = ecs.struct("Foo", "{float z;}")
local FooBar = ecs.struct("FooBar", "{Foo bar; float z;}")
local ents = ecs.bulk_new(10)

for i, e in ipairs(ents) do
    ecs.set(e, Position, { x = i * 10, y = i * 11})
    ecs.set(e, Velocity, { x = i * 12, y = i * 13})
end

ecs.singleton_set(AppContext, { 5000 })

local q = ecs.query("Position, Velocity, AppContext($)")
local it = ecs.query_iter(q)
local q_count = 0
local total = 0

while ecs.query_next(it) do
    local p, v, ctx = ecs.columns(it)
    q_count = q_count + 1

    assert(ctx.whatever == 5000)

    local j = total + 1
    for i = 1, it.count do
        print("p[i].x = " .. p[i].x)
        assert(p[i].x == j * 10)
        j = j + 1
    end

    total = total + it.count

    local i = 1
   --[[ for P, V, Ctx in ecs.each(it) do
      --  assert(P.x == i * 10)
        assert(Ctx.whatever == 5000)
        i = i + 1
    end]]--
end

print("count :" .. q_count)
u.asserteq(q_count, 10)

ecs.subquery(q, "Position")

total = 0

it = ecs.query_iter(q)

while ecs.query_next(it) do
    local p = ecs.columns(it)

    assert(p)
    total = total + it.count
end

u.asserteq(total, 10)


q = ecs.query("Position, Velocity")

q_count = 0
for p, v, e in ecs.each(q) do
    q_count = q_count + 1
end

print("each() count :" .. q_count)
assert(q_count >= 10)


--Empty query
q = ecs.query("Foo")

q_count = 0
for p, v, e in ecs.each(q) do
    q_count = q_count + 1
end

u.asserteq(q_count,  0)


ecs.bulk_new(FooBar, 5)

q = ecs.query("FooBar")

q_count = 0

for foo, e in ecs.each(q) do
    q_count = q_count + 1

    --make sure missing tables are recreated
    assert(foo.bar ~= nil)

    foo.bar = nil

end

u.asserteq(q_count, 5)
