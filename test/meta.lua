local ecs = require "ecs"
local t = require "test"
local u = require "util"

ecs.progress_cb(function() ecs.log("progress()!") end)

local tstruct = ecs.lookup("lua_test_struct")
local test_struct, added = ecs.get_mut(ecs.Singleton, tstruct)

--Verify values set by host
assert(not added)
assert(test_struct.b == true)
assert(test_struct.c == 1)
assert(test_struct.u8 == 2)
assert(test_struct.u16 == 4)
assert(test_struct.u32 == 8)
assert(test_struct.u64 == 16)
assert(test_struct.i8 == 32)
assert(test_struct.i16 == 64)
assert(test_struct.i32 == 128)
assert(test_struct.i64 == 256)
assert(test_struct.f32 == 512)
assert(test_struct.f64 == 1024)

assert(test_struct.ca[1] == 10 and test_struct.ca[4] == 40)

assert(test_struct.vptr == test_struct.uptr)

assert(test_struct.enumeration == 465)
assert(test_struct.bitmask == 1 | 2)

assert(test_struct.comp2.bar == 5)
assert(test_struct.comp.u16a[3] == 30)
assert(test_struct.comp2.comp.u16a[2] == 200)


--Modify values and check for roundtrip
test_struct.b = false
test_struct.i64 = 420
test_struct.comp2.comp.u16a[2] = 344
test_struct.enumeration = 500
test_struct.bitmask = (1 | 2 | 3)
test_struct.comp2.comp.foo = 1048333

ecs.modified(test_struct)
test_struct = nil
test_struct = ecs.get(ecs.Singleton, tstruct)

assert(test_struct.b == false)
assert(test_struct.i64 == 420)
assert(test_struct.vptr == test_struct.uptr)
assert(test_struct.comp2.comp.u16a[2] == 344)
assert(test_struct.enumeration == 500)
assert(test_struct.bitmask == (1 | 2 | 3))
assert(test_struct.comp2.comp.foo == 1048333)


--Partial declaration
assert(ecs.set(ecs.Singleton, tstruct, { i64 = 32}))
test_struct = ecs.get(ecs.Singleton, tstruct)
assert(test_struct.i64 == 32)

local v = { foo = "bar"}
assert(not pcall(function () ecs.modified(v) end))


