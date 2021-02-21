local t = require "test"
local ecs = require "ecs"
local u = require "util"

ecs.progress_cb(function() ecs.log("progress()!") end)

u.print_constants("MatchAll", "Module", "OnStore", "XOR")

local entity = ecs.new()
local only_id = ecs.new(4096)
local only_name = ecs.new("name_only")
local id_n_name = ecs.new(5120, "id_and_name")
local id_name_comp = ecs.new(10000, "with_comp", "lua_test_struct")
local no_id_comp = ecs.new("no_id", "lua_test_comp")
local id_comp = ecs.new(6000, nil, "lua_test_comp")
local just_comp = ecs.new(nil, "lua_test_comp")
local id_name_comps = ecs.new(10001, "multiple_comps", "lua_test_struct, lua_test_comp")
local bulk = ecs.bulk_new(10)
local bulk_comp = ecs.bulk_new("lua_test_comp", 10)
assert(ecs.bulk_new("lua_test_comp", 10, true) == nil)
assert(ecs.bulk_new(10, true) == nil)
local lua_test_comp = ecs.lookup("lua_test_comp")

local renamed = ecs.new("foo")
assert(ecs.name(renamed) == "foo")
assert(ecs.set_name(renamed, "bar") == renamed)
assert(ecs.name(renamed) == "bar")
assert(ecs.set_name(renamed, nil) == renamed)
assert(ecs.name(renamed) == nil)
assert(not pcall(function () ecs.set_name(renamed, false) end))

--last argument cannot be nil
assert(not pcall(function () ecs.new(nil) end))
assert(not pcall(function () ecs.new(0, nil) end))
assert(not pcall(function () ecs.new(0, "name", nil) end))

--name must be a string or nil
assert(not pcall(function () ecs.new(0, 0, "lua_test_comp") end))
assert(not pcall(function () ecs.new(0, true, "lua_test_comp") end))

--name must be a string if it's the last argument
assert(not pcall(function () ecs.new(0, 0) end))
assert(not pcall(function () ecs.new(0, true) end))

assert(not pcall(function () ecs.new(333, "too", "many", "args") end))
--invalid component strings cause a hard abort()
--assert(not pcall(function () ecs.new(0, "name", "no_such_component") end))

--existing entity redefined with different name
assert(not pcall(function () ecs.new(5120, "diff_name") end))

assert(ecs.new_id() ~= 0)
assert(only_id == 4096)
assert(ecs.name(only_id) == nil)
assert(ecs.name(only_name) == "name_only")
assert(id_n_name == 5120)
assert(id_comp == 6000)
assert(ecs.has(id_comp, lua_test_comp))
assert(just_comp ~= 0)
assert(ecs.name(just_comp) == nil)
assert(ecs.has(just_comp, lua_test_comp))
assert(ecs.name(id_n_name) == "id_and_name")
assert(ecs.lookup("ecs_lua_test_c_ent") == 8192)

assert(ecs.name(no_id_comp) == "no_id")
assert(ecs.has(no_id_comp, lua_test_comp))
assert(ecs.has(id_name_comps, lua_test_comp))
assert(ecs.name(id_name_comp) == "with_comp")
assert(ecs.name(id_name_comps) == "multiple_comps")

assert(#bulk == 10)
assert(ecs.has(bulk_comp[1], lua_test_comp))
assert(ecs.has(bulk_comp[10], lua_test_comp))
ecs.delete(bulk)
assert(not ecs.name(bulk[1]))
assert(not ecs.name(bulk[10]))

local type = ecs.get_type(lua_test_comp)
local invalid_type = ecs.get_type(bulk[1])
ecs.dim(400)
ecs.dim_type(400, type)
assert(not pcall(function () ecs.dim_type(400, invalid_type) end))


ecs.struct("LuaPosition", "{float x; float y; float z;}")
ecs.struct("LuaStruct", "{char blah[6]; LuaPosition position;}")

assert(not pcall(function() ecs.struct("LuaPosition", "{float x; float y; float z;}") end))

local alias = ecs.alias("LuaStruct", "LuaAlias")
assert(ecs.name(alias) == "LuaAlias")
ecs.add(alias, type)
ecs.add(alias, lua_test_comp)
assert(ecs.has(alias, lua_test_comp))
assert(not pcall(function() ecs.alias("error", "newname") end))
assert(not pcall(function() ecs.alias("name_only", "newname") end))
assert(not pcall(function() ecs.alias("LuaStruct", "LuaAlias") end))
assert(not pcall(function() ecs.alias("name_only", "LuaAlias") end))

local arr = ecs.array("LuaArray", "LuaStruct", 4)
assert(ecs.name(arr) == "LuaArray")

assert(not pcall(function() ecs.array("LuaArray", "LuaStruct", 4) end))

assert(not ecs.has(id_name_comp, arr))

assert(ecs.owns(id_name_comp, ecs.get_type(arr)) == false)
assert(ecs.owns(id_name_comp, arr) == false)

ecs.add(id_name_comp, arr)
assert(ecs.owns(id_name_comp, arr) == true)
--assert(ecs.owns(id_name_comp, ecs.get_type(arr)) == true)
assert(ecs.has(id_name_comp, arr))
assert(ecs.is_component_enabled(id_name_comp, arr) == true)

ecs.disable_component(id_name_comp, arr)
assert(ecs.is_component_enabled(id_name_comp, arr) == false)

ecs.enable_component(id_name_comp, arr)
assert(ecs.is_component_enabled(id_name_comp, arr) == true)

ecs.disable(id_name_comp, arr)
assert(ecs.is_component_enabled(id_name_comp, arr) == false)

ecs.enable(id_name_comp, arr)
assert(ecs.is_component_enabled(id_name_comp, arr) == true)

ecs.remove(id_name_comp, arr)
assert(not ecs.has(id_name_comp, arr))
assert(not pcall(function () ecs.has(id_name_comp, "does_not_exist") end))

local with_name = ecs.count(ecs.Name)
print("entities with EcsName: " .. with_name)

assert(with_name > 0)
assert(with_name == ecs.count(ecs.get_type(ecs.Name, true)))
assert(with_name == ecs.count({ include = ecs.get_type(ecs.Name, true)}))

local parent = ecs.new()
local child = 16666
ecs.add(child, ecs.CHILDOF | parent)

assert(ecs.get_parent(child) == parent)

ecs.clear(parent)
ecs.delete(child)

assert(ecs.exists(child))
assert(not ecs.is_alive(child))

local tag = ecs.tag("LuaTag")
local tag2 = ecs.tag("LuaTag2")

ecs.add(only_name, tag)
ecs.add(only_name, tag2)
ecs.add(id_name_comp, tag)

assert(ecs.has(only_name, tag))
assert(ecs.has(only_name, tag2))
assert(ecs.has(id_name_comp, tag))

ecs.remove(only_name, tag)
assert(not ecs.has(only_name, tag))
assert(ecs.has(id_name_comp, tag))

ecs.delete(tag2)
assert(pcall(function () ecs.delete(tag2) end)) -- is this normal?

assert(pcall(function () ecs.delete("LuaTag") end))
assert(not pcall(function () ecs.delete("LuaTag") end))
assert(not pcall(function () ecs.delete("LuaTag2") end))

--Test idempotence
tag = ecs.tag("Tag")
tag2 = ecs.tag("Tag2")
assert(tag == ecs.tag("Tag"))
assert(tag2 == ecs.tag("Tag2"))
assert(only_id == ecs.new(4096))
assert(only_name == ecs.new("name_only"))
assert(id_n_name == ecs.new(5120, "id_and_name"))
assert(id_name_comp == ecs.new(10000, "with_comp", "lua_test_struct"))
assert(no_id_comp == ecs.new("no_id", "lua_test_comp"))
assert(id_name_comps == ecs.new(10001, "multiple_comps", "lua_test_struct, lua_test_comp"))

assert(ecs.has(id_name_comps, ecs.lookup("lua_test_struct")))
ecs.clear(id_name_comps)
assert(not ecs.has(id_name_comps, ecs.lookup("lua_test_struct")))


--Enable/Disable
ecs.disable(entity)
assert(ecs.has(entity, ecs.Disabled))

ecs.enable(entity)
assert(not ecs.has(entity, ecs.Disabled))

ecs.add_instanceof(entity, tag)
ecs.remove_instanceof(entity, tag)

ecs.add_childof(entity, tag)
ecs.remove_childof(entity, tag)

ecs.add_owned(entity, tag)


--Lookup
local LuaWorldInfo = ecs.lookup_fullpath("flecs.lua.LuaWorldInfo")
assert(LuaWorldInfo ~= 0)

ecs.add(ecs.new("named"), ecs.CHILDOF | entity)
assert(ecs.lookup_child(entity, "named") ~= 0)

assert(ecs.lookup("lua_world_info") == 0)
ecs.use(LuaWorldInfo, "lua_world_info")
assert(ecs.lookup("lua_world_info") ~= 0)

local FlecsLua = ecs.lookup_symbol("flecs.lua")
assert(FlecsLua ~= 0)

assert(ecs.lookup_path(FlecsLua, "LuaWorldInfo") ~= 0)


--Bulk, filters

local ent = ecs.new(nil, "LuaPosition")

assert(ecs.is_alive(ent))

ecs.bulk_delete({ include = ecs.get_type(ent) })

assert(not ecs.is_alive(ent))


assert(not pcall(function () ecs.bulk_delete({include_kind = 0xD00D00}) end))
assert(not pcall(function () ecs.bulk_delete({exclude_kind = ecs.MatchExact + 1}) end))
assert(not pcall(function () ecs.bulk_delete({include_kind = false}) end))
assert(not pcall(function () ecs.bulk_delete({exclude_kind = false}) end))
assert(not pcall(function () ecs.bulk_delete({include = 0xD00D00}) end))
assert(not pcall(function () ecs.bulk_delete({exclude = 0xD00D00}) end))
