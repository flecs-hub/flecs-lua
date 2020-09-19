local ecs = require "ecs"

local m = {}

--These are declared for ease of use / autocomplete
m.random_id = 0
m.fixed_id = 0
m.name_only = 0

m.sys = 0

function m.HelloWorld()
    print("Hello World!")
end

m.imported = 0

ecs.module("test", function ()

    local private = ecs.new()

    m.random_id = ecs.new()
    m.fixed_id = ecs.new(16000, "named")
    m.name_only = ecs.new("rand_id")

    m.sys = ecs.system(m.HelloWorld, "HelloWorld", ecs.OnUpdate)

    assert(not pcall (function () ecs.system(m.random_id, "Func", ecs.OnUpdate) end))
    assert(m.imported == 0)

    m.imported = 1
end)

--ecs.module -> ecs_import() -> import_func(if not loaded) { ecs_module(); m.import() } ?

return m