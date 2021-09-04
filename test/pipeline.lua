local t = require "test"
local ecs = require "ecs"
local u = require "util"

u.test_defaults()

local P1 = ecs.pipeline("P1", "flecs.pipeline.OnUpdate, flecs.pipeline.PostUpdate")
local P2 = ecs.pipeline("P2", "flecs.pipeline.OnUpdate")

ecs.set_pipeline(P1)
assert(ecs.get_pipeline() == P1)

local Struct = ecs.struct("LuaStruct", "{int32_t x;}")

local e = ecs.set(ecs.new(), Struct, {x=1})

local sys_a_invoked = 0
local sys_b_invoked = 0


local function SysA(it)
    sys_a_invoked = sys_a_invoked + 1
    assert(ecs.get_thread_index() == 0)
end

local function SysB(it)
    sys_b_invoked = sys_b_invoked + 1
    assert(ecs.get_thread_index() == 0)
end

local sys_a = ecs.system(SysA, "SysA", ecs.OnUpdate, "LuaStruct")
local sys_b = ecs.system(SysB, "SysB", ecs.PostUpdate)


assert(ecs.get_threads ~= 0)
assert(ecs.get_stage_id() == 0)
assert(ecs.get_thread_index() == 0)

ecs.set_time_scale(2)
ecs.set_target_fps(60)

ecs.progress(0)
assert(sys_a_invoked == 1)

sys_a_invoked = 0
sys_b_invoked = 0
ecs.set_pipeline(P2)
assert(ecs.get_pipeline() == P2)

ecs.progress(0)
assert(sys_a_invoked == 1)
assert(sys_b_invoked == 0)

local wi = ecs.world_info()

assert(wi.world_time_total ~= 0)

ecs.reset_clock()

wi = ecs.world_info()

assert(wi.world_time_total == 0)

assert(not ecs.has(sys_a, ecs.Inactive))

ecs.delete(e)

ecs.deactivate_systems()

assert(ecs.has(sys_a, ecs.Inactive))
assert(not ecs.has(sys_b, ecs.Inactive))