local ecs = require "ecs"
local t = require "test"
local u = require "util"

ecs.progress(function() ecs.log("progress()!") end)

local UINT8_MAX  = 0xff
local UINT16_MAX = 0xffff
local UINT32_MAX = 0xffffffff
local UINT64_MAX = 0xffffffffffffffff

local INT8_MAX   = 127
local INT16_MAX  = 32767
local INT32_MAX  = 2147483647
local INT64_MAX  = 9223372036854775807

local INT8_MIN   = -127 - 1
local INT16_MIN  = -32767 - 1
local INT32_MIN  = -2147483647 - 1
local INT64_MIN  = -9223372036854775807

local F32_MAX = (1 << 24) - 1
local F64_MAX = (1 << 53) - 1
local F32_MIN = -(1 << 24)
local F64_MIN = -(1 << 53)

local tstruct = ecs.lookup("lua_test_struct")
local test_struct, added = ecs.get_mut(ecs.Singleton, tstruct)


--Verify values set by host
assert(not added)
assert(test_struct.b == true)
assert(test_struct.c == 1)
assert(test_struct.ca[1] == 10 and test_struct.ca[4] == 40)
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
assert(test_struct.comp2.comp.u16a[2] == 344)
assert(test_struct.enumeration == 500)
assert(test_struct.bitmask == (1 | 2 | 3))
assert(test_struct.comp2.comp.foo == 1048333)


--Partial declaration
assert(ecs.set(ecs.Singleton, tstruct, { i64 = 32}))
test_struct = ecs.get(ecs.Singleton, tstruct)
assert(test_struct.i64 == 32)


--Roundtrip with max values
test_struct = ecs.get_mut(ecs.Singleton, tstruct)

test_struct.c = INT8_MAX
test_struct.u8 = UINT8_MAX
test_struct.u16 = UINT16_MAX
test_struct.u32 = UINT32_MAX
test_struct.u64 = UINT64_MAX
test_struct.i8 = INT8_MAX
test_struct.i16 = INT16_MAX
test_struct.i32 = INT32_MAX
test_struct.i64 = INT64_MAX
test_struct.f32 = F32_MAX
test_struct.f64 = F64_MAX
test_struct.enumeration = INT32_MAX
test_struct.bitmask = INT32_MAX

ecs.modified(test_struct)
test_struct = nil
test_struct = ecs.get_mut(ecs.Singleton, tstruct)

assert(test_struct.c == INT8_MAX)
assert(test_struct.u8 == UINT8_MAX)
assert(test_struct.u16 == UINT16_MAX)
assert(test_struct.u32 == UINT32_MAX)
assert(test_struct.u64 == UINT64_MAX)
assert(test_struct.i8 == INT8_MAX)
assert(test_struct.i16 == INT16_MAX)
assert(test_struct.i32 == INT32_MAX)
assert(test_struct.i64 == INT64_MAX)
assert(test_struct.f32 == F32_MAX)
assert(test_struct.f64 == F64_MAX)
assert(test_struct.enumeration == INT32_MAX)
assert(test_struct.bitmask == INT32_MAX)


--Roundtrip with minimum/negative values
test_struct = ecs.get_mut(ecs.Singleton, tstruct)

test_struct.u8 = -1
test_struct.u16 = -1
test_struct.u32 = -1
test_struct.u64 = -1
test_struct.i8 = INT8_MIN
test_struct.i16 = INT16_MIN
test_struct.i32 = INT32_MIN
test_struct.i64 = INT64_MIN
test_struct.f32 = F32_MIN
test_struct.f64 = F64_MIN

ecs.modified(test_struct)
test_struct = nil
test_struct = ecs.get_mut(ecs.Singleton, tstruct)

assert(test_struct.u8 == UINT8_MAX)
assert(test_struct.u16 == UINT16_MAX)
assert(test_struct.u32 == UINT32_MAX)
assert(test_struct.u64 == UINT64_MAX)
assert(test_struct.i8 == INT8_MIN)
assert(test_struct.i16 == INT16_MIN)
assert(test_struct.i32 == INT32_MIN)
assert(test_struct.i64 == INT64_MIN)
assert(test_struct.f32 == F32_MIN)
assert(test_struct.f64 == F64_MIN)

--Out of range
--assert(not pcall(function () ecs.set(ecs.Singleton, tstruct, { u8 = INT8_MIN - 1}) end))
--assert(not pcall(function () ecs.set(ecs.Singleton, tstruct, { u16 = INT16_MIN - 1}) end))
assert(not pcall(function () ecs.set(ecs.Singleton, tstruct, { u32 = INT32_MIN - 1}) end))
assert(not pcall(function () ecs.set(ecs.Singleton, tstruct, { f32 = F32_MAX + 1}) end))
assert(not pcall(function () ecs.set(ecs.Singleton, tstruct, { f32 = F32_MIN - 1}) end))
assert(not pcall(function () ecs.set(ecs.Singleton, tstruct, { i16 = 1000000}) end))