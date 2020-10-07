local ecs = require "ecs"
local t = require "test"
local u = require "util"

assert(ecs.assert(123) == 123)

u.print_constants("MatchAll", "Module", "OnStore", "XOR")

local entity = ecs.new()
local only_id = ecs.new(4096)
local only_name = ecs.new("name_only")
local id_n_name = ecs.new(5120, "id_and_name")
local id_name_comp = ecs.new(10000, "with_comp", "lua_test_struct")
local no_id_comp = ecs.new("no_id", "lua_test_comp")
local id_name_comps = ecs.new(10001, "multiple_comps", "lua_test_struct, lua_test_comp")
local bulk = ecs.bulk_new(10)
local bulk_comp = ecs.bulk_new("lua_test_comp", 10)

u.print_entities(entity, only_id, only_name, id_n_name, 8192)

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

assert(#bulk == 10)
assert(ecs.has(bulk_comp[1], "lua_test_comp"))
assert(ecs.has(bulk_comp[10], "lua_test_comp"))
assert(ecs.name(bulk[1]):starts("Lua"))
ecs.delete(bulk)
assert(not ecs.name(bulk[1]))
assert(not ecs.name(bulk[10]))

ecs.struct("LuaPosition", "{float x; float y; float z;}")
ecs.struct("LuaStruct", "{char blah[6]; LuaPosition position;}")

local alias = ecs.alias("LuaStruct", "LuaAlias")
assert(ecs.name(alias) == "LuaAlias")
ecs.add(alias, "lua_test_comp")
assert(ecs.has(alias, "lua_test_comp"))
assert(not pcall(function() ecs.alias("error", "newname") end))
assert(not pcall(function() ecs.alias("name_only", "newname") end))

local arr = ecs.array("LuaArray", "(LuaStruct,4)")
assert(ecs.name(arr) == "LuaArray")

assert(not ecs.has(id_name_comp, "LuaArray"))

ecs.add(id_name_comp, arr)
assert(ecs.has(id_name_comp, arr))

ecs.remove(id_name_comp, arr)
assert(not ecs.has(id_name_comp, arr))
assert(not pcall(function () ecs.has(id_name_comp, "does_not_exist") end))

local parent = ecs.new()
local child = ecs.new(ecs.CHILDOF | parent)

assert(ecs.has_role(child, ecs.CHILDOF))
assert(not ecs.has_role(parent, ecs.CHILDOF))

ecs.clear(parent)
ecs.delete(child)

assert(ecs.exists(child))
assert(not ecs.is_alive(child))

local tag = ecs.tag("LuaTag")
local tag2 = ecs.tag("LuaTag2")

ecs.add(only_name, tag)
ecs.add(only_name, "LuaTag2")
ecs.add(id_name_comp, "LuaTag")

assert(ecs.has(only_name, "LuaTag"))
assert(ecs.has(only_name, "LuaTag2"))
assert(ecs.has(id_name_comp, "LuaTag"))

ecs.remove(only_name, tag)
assert(not ecs.has(only_name, "LuaTag"))
assert(ecs.has(id_name_comp, "LuaTag"))

ecs.delete(tag2)
assert(pcall(function () ecs.delete(tag2) end)) -- is this normal?
assert(not pcall(function () ecs.has(only_name, "LuaTag2") end))

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

assert(ecs.has(id_name_comps, "lua_test_struct"))
ecs.clear(id_name_comps)
assert(not ecs.has(id_name_comps, "lua_test_struct"))

local m = require "modules.test"
--u.print_packages()
local m2 = require "modules.test"

assert(m.imported == 1)
assert(m.fixed_id == 16000)

--Sanity check
assert(m.random_id == m2.random_id)
assert(m.fixed_id == m2.fixed_id)
assert(m.name_only == m2.name_only)

assert(ecs.lookup_fullpath("flecs.lua.LuaWorldInfo") ~= 0)
assert(ecs.lookup_fullpath("test.MStruct") ~= 0)
assert(ecs.lookup_fullpath("test.MStruct") == m.component)
assert(ecs.lookup_fullpath("test.NoSuchComponent") == 0)

ecs.set_target_fps(60)

local wi = ecs.world_info()
ecs.log("target fps: " .. wi.target_fps, "last component: " .. wi.last_component_id)

ecs.progress_cb(function()
    ecs.log("progress()!")
end)
