local ecs = require "ecs"

ecs.progress_cb(function () end)

local Position = ecs.struct("Position", "{float x; float y;}")

local Destroyer = ecs.prefab("Destroyer")

    local FrontTurret = ecs.prefab("FrontTurret", "CHILDOF | Destroyer, Position")
        ecs.set(FrontTurret, Position, { -10, 0 })

    local BackTurret = ecs.prefab("BackTurret", "CHILDOF | Destroyer, Position")
        ecs.set(BackTurret, Position, { 10, 0 })

local inst = ecs.new(ecs.INSTANCEOF | Destroyer)

assert(ecs.has(Destroyer, ecs.Prefab))

local Watergun = ecs.prefab()

assert(ecs.has(Watergun, ecs.Prefab))


assert(not pcall(function () ecs.prefab("blah", "Position", "test") end))