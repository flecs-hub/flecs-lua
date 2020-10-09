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



assert(ecs.lookup_fullpath("test.MStruct") ~= 0)
assert(ecs.lookup_fullpath("test.MStruct") == m.component)
assert(ecs.lookup_fullpath("test.NoSuchComponent") == 0)