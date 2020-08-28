local ecs = require "ecs"
local t = require "test"

local entity = ecs.new()
local only_id = ecs.new(4096)
local only_name = ecs.new("name_only")
local id_n_name = ecs.new(5120, "id_and_name")
local id_name_comp = ecs.new(10000, "with_comp", "lua_test_struct")
local no_id_comp = ecs.new("no_id", "lua_test_comp")
local id_name_comps = ecs.new(10001, "multiple_comps", "lua_test_struct, lua_test_comp")

local function print_entity(...)
    local args = {...}
    for i,v in ipairs(args) do
        print("id: " .. v .. ", name: " .. ecs.name(v))
    end
end

print("Entities:")

print_entity(entity, only_id, only_name, id_n_name, 8192)

assert(only_id == 4096)
assert(ecs.name(only_id) ~= nil)
assert(ecs.name(only_name) == "name_only")
assert(id_n_name == 5120)
assert(ecs.name(id_n_name) == "id_and_name")
assert(not pcall(function () ecs.new(nil) end))
assert(not pcall(function () ecs.new(0, nil) end))
assert(not pcall(function () ecs.new(333, "too", "many", "args") end))
assert(ecs.lookup("ecs_lua_test_c_ent") == 8192)

assert(ecs.name(no_id_comp) == "no_id")
assert(ecs.has(no_id_comp, "lua_test_comp"))
assert(ecs.has(id_name_comps, "lua_test_comp"))
assert(ecs.name(id_name_comp) == "with_comp")
assert(ecs.name(id_name_comps) == "multiple_comps")

ecs.struct("LuaPosition", "{float x; float y; float z;}")
ecs.struct("LuaStruct", "{char blah[6]; LuaPosition position;}")
local arr = ecs.array("LuaArray", "(LuaStruct,4)")
assert(ecs.name(arr) == "LuaArray")

assert(not ecs.has(id_name_comp, "LuaArray"))

ecs.add(id_name_comp, arr)
assert(ecs.has(id_name_comp, arr))

ecs.remove(id_name_comp, arr)
assert(not ecs.has(id_name_comp, arr))

ecs.add(id_name_comp, "LuaArray")
assert(ecs.has(id_name_comp, "LuaArray"))

ecs.remove(id_name_comp, "LuaArray")
assert(not ecs.has(id_name_comp, "LuaArray"))