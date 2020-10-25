--EmmyLua annotations and documentation for flecs-lua
--NOTE: ecs is a native module, this is not used at runtime.

local ecs = {}

---@class ecs_type_t
local ecs_type_t = {}

---@class ecs_filter_t
---@field include ecs_type_t
---@field exclude ecs_type_t
---@field include_kind integer @ecs.Match*
---@field exclude_kind integer @ecs.Match*
local ecs_filter_t = {}

---@class ecs_mutable_t
local ecs_mutable_t = {}

---@class ecs_query_t
local ecs_query_t = {}

---@class ecs_iter_t
---@field count integer
---@field columns table[]
---@field system integer

---@field table_count integer
---@field table_offset integer

---@field delta_time number
---@field delta_system_time number
---@field world_time number
local ecs_iter_t = {}

---Create a new entity
---@param e integer
---@param name string
---@param components string
---@return integer @entity
function ecs.new(e, name, components)
end

--trailing arguments can be ommitted
ecs.new()
ecs.new(123)
ecs.new(123, "name")

--entity id can be ommited
ecs.new("name")
ecs.new("name", "component")
ecs.new(0, "name", "component1, component2")

--name can be nil
ecs.new(nil, "just_component")
ecs.new(123, nil, "id, components")

--the last argument cannot be nil
ecs.new(nil) --invalid
ecs.new(123, nil) --invalid
ecs.new(123, "name", nil) --invalid
ecs.new(123, 0, "component") --invalid: name must be a string or nil


---Delete an entity and all of its components
---@param entity string|integer|integer[]
function ecs.delete(entity)
end

---Create a tag
---@param name integer
---@return integer @entity
function ecs.tag(name)
end

---Get the name of an entity
---@param e integer
---@return string|nil @entity name
function ecs.name(e)
end

---Look up an entity by name
---@param name string
---@return string @entity name
function ecs.lookup(name)
end

---Lookup an entity from a full path
---@param name string
---@return string @entity name
function ecs.lookup_fullpath(name)
end

---Test if an entity has a component, type or tag
---@param e integer @entity
---@param type string|integer
---@return boolean
function ecs.has(e, type)
end

---Test if an entity has a role
---@param e integer @entity
---@param role integer
---@return boolean
function ecs.has_role(e, role)
end

---Test whether an entity is alive
---@param e integer entity
---@return boolean
function ecs.is_alive(e)
end

---Test whether an entity exists
---Similar to ecs.is_alive, but ignores entity generation count
---@param e integer entity
---@return boolean
function ecs.exists(e)
end

---Add a component, type or tag to an entity
---@param e integer entity
---@param type string|integer
function ecs.add(e, type)
end

---Remove a component, type or tag from an entity
---@param e integer @entity
---@param type string|integer
function ecs.remove(e, type)
end

---Clear all components
---@param e integer entity
function ecs.clear(e)
end

---Get the type of an entity
---@param e integer entity
---@return ecs_type_t|nil
function ecs.get_type(e)
end

---Create a meta array component
---@param name string name
---@param type string
---@param count integer
---@return integer @entity
function ecs.array(name, type, count)
end

---Create a meta struct component
---@param name string name
---@param descriptor string @format: "{type member; ...}"
---@return integer @entity
function ecs.struct(name, descriptor)
end

---Create an alias for a meta type
---@param meta_type string @name
---@param alias string
---@return integer @entity
function ecs.alias(meta_type, alias)
end

---Get an immutable table to a component
---@param entity integer
---@param component integer
---@return table
function ecs.get(entity, component)
end

---Get a mutable table to a component
---@param entity integer
---@param component integer
---@return ecs_mutable_t, boolean
function ecs.get_mut(entity, component)
end

---Signal that a component has been modified
---@param t ecs_mutable_t
function ecs.modified(t)
end

---Set the value of a component
---@param entity integer
---@param component integer
---@param v table
---@return integer entity
function ecs.set(entity, component, v)
end

---Get an immutable table to a singleton component
---@param component integer
---@return table
function ecs.singleton_get(component)
end

---Get a mutable table to a singleton component
---@param component integer
---@return ecs_mutable_t, boolean
function ecs.singleton_get_mut(component)
end

---Signal that a singleton component has been modified
---@param component ecs_mutable_t
function ecs.singleton_modified(component)
end

---Set the value of a singleton component
---@param component integer
---@param value table
---@return integer entity
function ecs.singleton_set(component, value)
end

---Create a prefab
---@param name string
---@param signature string
---@return integer entity
function ecs.prefab(name, signature)
end

---Create N new entities with an optional component
---@param type integer|string @optional
---@param n integer
---@return integer[]
function ecs.bulk_new(type, n)
end

---Delete entities matching a filter
---@param filter ecs_filter_t
function ecs.bulk_delete(filter)
end

---Get column from iterator
---@param it ecs_iter_t
---@param col integer
---@return table
function ecs.column(it, col)
end

---Returns all columns
---@param it ecs_iter_t
---@return table
function ecs.columns(it)
end

---Test if column is owned
---@param it ecs_iter_t
---@param column integer
function ecs.is_owned(it, column)
end

---Create a query
---@param sig string
---@return ecs_query_t
function ecs.query(sig)
end

---Create a query iterator
---@param query ecs_query_t
---@return ecs_iter_t
function ecs.query_iter(query)
end

---Progress the query iterator
---@param it ecs_iter_t
---@return boolean
function ecs.query_next(it)
end

---Check whether the query data changed since last iteration
---@param query ecs_query_t
---@return boolean
function ecs.query_changed(query)
end

---Create a system
---@param func function
---@param name string
---@param phase integer
---@param signature string @optional
---@return integer @entity
function ecs.system(func, name, phase, signature)
end

---Create a module
---@param name string
---@param cb function import callback
---@return integer @entity
function ecs.module(name, cb)
end

---Print log message
function ecs.log(...)
end

---Print error message
function ecs.err(...)
end

---Print debug message
function ecs.dbg(...)
end

---Print warning message
function ecs.warn(...)
end

---Enable or disable tracing
---@param level integer
function ecs.tracing_enable(level)
end

---Similar to the standard assert(), but the assertion
---will always succeed when built with -DNDEBUG
---@param v any
---@param message any
function ecs.assert(v, message)
end

---@class ecs_time_t
---@field sec integer
---@field nanosec integer
local ecs_time_t = {}

---Get the current time
---@return ecs_time_t
function ecs.get_time()
end

---Measure elapsed time for timestamp
---@param start ecs_time_t
---@return number
function ecs.time_measure(start)
end

---Create a new pipeline
---@param e integer
---@param name string
---@param expr string
---@return integer
function ecs.pipeline(e, name, expr)
end

---Set a custom pipeline
---@param pipeline integer
function ecs.set_pipeline(pipeline)
end

---Get the current pipeline
---@return integer
function ecs.get_pipeline()
end

---Progress the world
---@param delta_time number
---@return boolean
function ecs.progress(delta_time)
end

---Set progress function callback
---@param cb function
function ecs.progress_cb(cb)
end

---Set target fps
---@param fps number
function ecs.set_target_fps(fps)
end

---Set time scale
---@param scale number
function ecs.set_time_scale(scale)
end

---Reset world clock
function ecs.reset_clock()
end

---Signal that the application should quit
function ecs.quit()
end

---Deactivate systems that are not matched with tables
function ecs.deactivate_systems()
end

---Set number of worker threads
---@param threads integer
function ecs.set_threads(threads)
end

---Get current number of threads
---@return integer
function ecs.get_threads()
end

---Get current thread index
---@return integer
function ecs.get_thread_index()
end


---@class world_info_t
---@field last_component_id integer
---@field last_id integer
---@field min_id integer
---@field max_id integer

---@field delta_time_raw number
---@field delta_time number
---@field time_scale number
---@field target_fps number
---@field frame_time_total number
---@field system_time_total number
---@field merge_time_total number
---@field world_time_total number
---@field world_time_total_raw number
---@field sleep_err number

---@field frame_count_total integer
---@field merge_count_total integer
---@field pipeline_build_count_total integer
---@field systems_ran_frame integer
local world_info_t = {}

---Get world info
---@return world_info_t
function ecs.world_info()
end

---Dimension the world for a specified number of entities
---@param count integer entity
function ecs.dim(count)
end

---Dimension a type for a specified number of entities
---@param count integer entity
---@param type ecs_type_t
function ecs.dim_type(count, type)
end

---Create an EmmyLua class annotation
---@param type_entity integer
---@param struct_as_table boolean @optional, print member structs' type as table
---@return string
function ecs.emmy_class(type_entity, struct_as_table)
end

---@class EcsMetaType
---@field kind integer
---@field size integer
---@field alignment integer
---@field descriptor string
---@field alias integer
local EcsMetaType = {}

---@class ecs_type_op_t
---@field type integer
---@field kind integer
---@field size integer
---@field alignment integer
---@field count integer
---@field offset integer
---@field name string
local ecs_type_op_t = {}

---@class EcsMetaTypeSerializer
---@field ops ecs_type_op_t[]
local EcsMetaTypeSerializer = {}


ecs.MatchDefault = 0
ecs.MatchAll = 1
ecs.MatchAny = 2
ecs.MatchExact = 3
ecs.Module = 256
ecs.Prefab = 257
ecs.Hidden = 258
ecs.Disabled = 259
ecs.DisabledIntern = 260
ecs.Inactive = 261
ecs.OnDemand = 262
ecs.Monitor = 263
ecs.Pipeline = 264
ecs.OnAdd = 265
ecs.OnRemove = 266
ecs.OnSet = 267
ecs.UnSet = 268
ecs.PreFrame = 269
ecs.OnLoad = 270
ecs.PostLoad = 271
ecs.PreUpdate = 272
ecs.OnUpdate = 273
ecs.OnValidate = 274
ecs.PostUpdate = 275
ecs.PreStore = 276
ecs.OnStore = 277
ecs.PostFrame = 278
ecs.Flecs = 279
ecs.FlecsCore = 280
ecs.World = 281
ecs.Singleton = 282
ecs.Wildcard = 283
ecs.INSTANCEOF = -144115188075855872
ecs.CHILDOF = -216172782113783808
ecs.TRAIT = -288230376151711744
ecs.AND = -360287970189639680
ecs.OR = -432345564227567616
ecs.XOR = -504403158265495552
ecs.NOT = -576460752303423488
ecs.CASE = -648518346341351424
ecs.SWITCH = -720575940379279360
ecs.OWNED = -792633534417207296

return ecs