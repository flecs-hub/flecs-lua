local ecs = require "ecs"

ecs.progress_cb(function() ecs.log("progress()!") end)

local m = require "modules.test"
--u.print_packages()
local m2 = require "modules.test"

assert(m.imported == 1)
assert(m.fixed_id == 16000)

--Sanity check
assert(m.random_id == m2.random_id)
assert(m.fixed_id == m2.fixed_id)
assert(m.name_only == m2.name_only)


assert(ecs.lookup_fullpath("lua.test.MStruct") ~= 0)
assert(ecs.lookup_fullpath("lua.test.MStruct") == m.component)
assert(ecs.lookup_fullpath("lua.test.NoSuchComponent") == 0)

assert(ecs.get_child_count(m.id) ~= 0)

local scope = ecs.get_scope()
assert(scope == 0)
scope = ecs.set_scope(m.id)
assert(scope == 0)
scope = ecs.get_scope()
assert(scope == m.id)

local prev = ecs.set_name_prefix("tst")
assert(prev == nil)

prev = ecs.set_name_prefix(nil)
assert(prev == "tst")

local it = ecs.scope_iter(m.id)

while ecs.scope_next(it) do
    print("it.count: " .. it.count)
    assert(it.count ~= 0)
end