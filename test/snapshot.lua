local ecs = require "ecs"
local u = require "util"

ecs.progress_cb(function() ecs.log("progress()!") end)


local struct = ecs.struct("Struct", "{int32_t x;}")

ecs.bulk_new(struct, 10)

local snapshot = ecs.snapshot()

local filter =
{
    include = ecs.get_type(struct)
}

local it = ecs.snapshot_iter(snapshot, filter)

local b = false

while ecs.snapshot_next(it) do
    b = true

end

--assert(b == true)