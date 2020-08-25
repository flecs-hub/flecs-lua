local ecs = require "ecs"
local t = require "test"

local e = ecs.new()
local name = ecs.name(e)

print("entity id: ", e, ", name: ", name)



