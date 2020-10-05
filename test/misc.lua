local ecs = require "ecs"

ecs.progress(function() ecs.log("progress()!") end)

ecs.log("This is a LOG message: ", 1234, ecs.OnStore)
ecs.err("This is an ERROR message")
ecs.dbg("This is a DEBUG message")
ecs.warn("This is a WARN message")

ecs.assert(1)
ecs.assert(1, "test")
assert(not pcall(function () ecs.assert(false, "failing assert") end))