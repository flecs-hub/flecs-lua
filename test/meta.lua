local ecs = require "ecs"
local t = require "test"
local u = require "util"

ecs.progress_cb(function() ecs.log("progress()!") end)

local tstruct = ecs.lookup("lua_test_struct")
local test_struct, added = ecs.singleton_get_mut(tstruct)

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

assert(test_struct.str == "test string")
assert(test_struct.vptr == test_struct.uptr)

assert(test_struct.enumeration == 465)
assert(test_struct.bitmask == 1 | 2)

assert(test_struct.comp2.bar == 5)
assert(test_struct.comp.u16a[3] == 30)
assert(test_struct.comp2.comp.u16a[2] == 200)


--Modify values and check for roundtrip
test_struct.b = false
test_struct.i64 = 420
test_struct.str = "lua string"
test_struct.comp2.comp.u16a[2] = 344
test_struct.enumeration = 500
test_struct.bitmask = (1 | 2 | 3)
test_struct.comp2.comp.foo = 1048333

ecs.singleton_modified(tstruct, test_struct)
test_struct = nil
test_struct = ecs.singleton_get(tstruct)

assert(test_struct.b == false)
assert(test_struct.i64 == 420)
assert(test_struct.str == "lua string")
assert(test_struct.vptr == test_struct.uptr)
assert(test_struct.comp2.comp.u16a[2] == 344)
assert(test_struct.enumeration == 500)
assert(test_struct.bitmask == (1 | 2 | 3))
assert(test_struct.comp2.comp.foo == 1048333)


test_struct = ecs.singleton_get_mut(tstruct)

--Free string by setting it to 0
test_struct.str = 0
ecs.singleton_modified(tstruct, test_struct)
test_struct = ecs.singleton_get(tstruct)
assert(test_struct.str == nil) --NULL strings are not serialized


--Partial declaration
assert(ecs.singleton_set(tstruct, { i64 = 32}))
test_struct = ecs.singleton_get(tstruct)
assert(test_struct.i64 == 32)


local MetaType = ecs.lookup_fullpath("flecs.meta.MetaType")
local MetaTypeSerializer = ecs.lookup_fullpath("flecs.meta.EcsMetaTypeSerializer")
local ecs_type_op = ecs.lookup_fullpath("flecs.meta.ecs_type_op_t")

local LuaEnum = ecs.enum("LuaEnum", "{ Red, Green, Blue = 4 }")
local LuaBitmask = ecs.bitmask("LuaBitmask", "{ Bacon = 1, Lettuce = 2, Tomato = 4 }")
local TestStruct = ecs.struct("TestStruct", "{ int32_t x; LuaEnum enum; LuaBitmask mask; }")

local LuaPosition = ecs.struct("LuaPosition", "{float x; float y; float z;}")
local LuaStruct = ecs.struct("LuaStruct", "{ uint8_t blah[6]; LuaPosition position;}")
local LuaArray = ecs.array("LuaArray", "LuaStruct", 4)

---@type EcsMetaType
local meta = ecs.get(LuaStruct, MetaType)

---@type EcsMetaTypeSerializer
local ser = ecs.get(LuaStruct, MetaTypeSerializer)

local ref = ecs.ref(ecs.Singleton, tstruct)

test_struct = ecs.get_ref(ref)
assert(test_struct ~= nil)

test_struct = ecs.get_ref(ref, ecs.Singleton, tstruct)
assert(test_struct ~= nil)

assert(not pcall(function () ecs.get_ref(ref, ecs.Singleton) end))


for i, op in pairs(ser.ops) do
  print(string.format("ops[%s].kind = %d", i, op.kind))
end

local sizeof_LuaStruct = meta.size
assert(sizeof_LuaStruct == ecs.sizeof(LuaStruct))

meta = ecs.get(LuaArray, MetaType)

assert(meta.size == sizeof_LuaStruct * 4)

local data =
{
  [1] = { blah = { 100 }, position = {  10, 20, 30 } },
  [2] = { { 111 }, { 11, 22, 33 }},
}

ecs.set(ecs.Singleton, LuaArray, data)

data = ecs.get(ecs.Singleton, LuaArray)

assert(data[1].blah[1] == 100)
assert(data[2].blah[1] == 111)
assert(data[1].position.y == 20)
assert(data[2].position.y == 22)

--Structs without labels are allowed
ecs.set(ecs.Singleton, LuaPosition, { 100, 200, 300 })

--Mixed key types are not allowed
assert(not pcall(function () ecs.set(ecs.Singleton, LuaPosition, { 100, x = 2, 200, 300}) end))

--Too many elements
assert(not pcall(function () ecs.set(ecs.Singleton, LuaPosition, { 100, 200, 300, 400 }) end))

--Invalid key type
local lol = {}
lol[ecs.get_type(LuaPosition)] = 245
assert(not pcall(function () ecs.set(ecs.Singleton, LuaPosition, lol) end))

local str = ecs.emmy_class(LuaPosition)
ecs.log(str)