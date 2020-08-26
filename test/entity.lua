local ecs = require "ecs"
local t = require "test"

local entity = ecs.new()
local name = ecs.name(entity)

local only_id = ecs.new(4096)
local only_name = ecs.new("name_only")
local id_n_name = ecs.new(5120, "id_and_name")

local function print_entity(...)
    local args = {...}
    for i,v in ipairs(args) do
        print("id: " .. v .. ", name: " .. ecs.name(v))
    end
end

print("Entities:")

print_entity(entity, only_id, only_name, id_n_name, 8192)

assert(only_id == 4096)
assert(ecs.name(only_name) == "name_only")
assert(id_n_name == 5120)
assert(ecs.name(id_n_name) == "id_and_name")
assert(not pcall(ecs.new(), nil))
assert(not pcall(ecs.new(), 0, nil))
assert(ecs.lookup("ecs_lua_test_c_ent") == 8192)