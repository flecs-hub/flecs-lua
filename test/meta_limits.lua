local ecs = require "ecs"
local u = require "util"

ecs.progress_cb(function() ecs.log("progress()!") end)

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

local F32_MAX    =  (1 << 24) - 1
local F64_MAX    =  (1 << 53) - 1
local F32_MIN    = -(1 << 24)
local F64_MIN    = -(1 << 53)

local tstruct = ecs.lookup("lua_test_struct")
local test_struct, added = ecs.get_mut(ecs.Singleton, tstruct)


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

ecs.modified(ecs.Singleton, tstruct, test_struct)
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

ecs.modified(ecs.Singleton, tstruct, test_struct)
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
