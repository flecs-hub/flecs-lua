--EmmyLua annotations and documentation for flecs-lua
--NOTE: ecs is a native module, this is not used at runtime.

local ecs = {}

---@class ecs_type_t
local ecs_type_t = {}

---@class ecs_ref_t
local ecs_ref_t = {}

---@class ecs_term_set_t
---@field relation integer
---@field mask integer
---@field min_depth integer
---@field max_depth integer
local ecs_term_set_t = {}

---@class ecs_term_id_t
---@field entity integer
---@field var integer
---@field set ecs_term_set_t
local ecs_term_id_t = {}

---@class ecs_term_t
---@field id integer
---@field inout integer
---@field pred ecs_term_id_t
---@field subj ecs_term_id_t
---@field obj ecs_term_id_t
---@field oper integer
---@field role integer
local ecs_term_t = {}

---@class ecs_filter_t
---@field terms ecs_term_t|ecs_term_t[]
---@field expr string
local ecs_filter_t = {}

---@class ecs_query_t
local ecs_query_t = {}

---@class ecs_snapshot_t
local ecs_snapshot_t = {}

---@class ecs_iter_t
---@field count integer
---@field system integer
---@field event integer
---@field event_id integer
---@field self integer
---@field columns table[]
---@field entities integer[]
---@field table_count integer
---@field delta_time number
---@field delta_system_time number
---@field world_time number
---@field interrupted_by integer
---@field term_index integer
local ecs_iter_t = {}

---Create a new entity
---@param entity integer
---@param name string
---@param components string
---@return integer @entity
function ecs.new(entity, name, components)
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
ecs.new(123, nil, "component1, component2")

--the last argument cannot be nil
ecs.new(nil) --invalid
ecs.new(123, nil) --invalid
ecs.new(123, "name", nil) --invalid
ecs.new(123, 0, "component") --invalid: name must be a string or nil

---Create a new entity id without scope
---@return integer
function ecs.new_id()
end

---Delete an entity and all of its components
---@overload fun(entities: integer[])
---@param entity integer
function ecs.delete(entity)
end

---Create a tag
---@param name string
---@return integer @entity
function ecs.tag(name)
end

---Get the name of an entity
---@param entity integer
---@return string|nil @entity name
function ecs.name(entity)
end

---Set the name of an entity
---@param entity integer
---@param name string
---@return integer @entity id
function ecs.set_name(entity, name)
end

---Get the symbol name of an entity
---@param entity integer
---@return string|nil @entity symbol name
function ecs.symbol(entity)
end

---Get the full path for an entity
---@param entity integer
---@return string|nil
function ecs.fullpath(entity)
end

---Look up an entity by name
---@param name string
---@return string @entity name
function ecs.lookup(name)
end

---Look up a child entity by name
---@param entity integer
---@param name string
---@return integer
function ecs.lookup_child(entity, name)
end

---Lookup an entity from a path
---@param parent integer
---@param path string
---@param sep string @optional
---@param prefix string @optional
---@return integer
function ecs.lookup_path(parent, path, sep, prefix)
end

---Lookup an entity from a full path
---@param name string
---@return string @entity name
function ecs.lookup_fullpath(name)
end

---Look up an entity by its symbol name
---@param name string
---@return integer
function ecs.lookup_symbol(name)
end

---Add alias for entity to global scope
---@param entity integer
---@param name string
function ecs.use(entity, name)
end

---Test if an entity has a component, type or tag
---@overload fun(subject: integer, relation: integer, object: integer)
---@overload fun(entity: integer, to_check: ecs_type_t)
---@param entity integer
---@param to_check integer
---@return boolean
function ecs.has(entity, to_check)
end

---Test if an entity owns a component
---@param entity integer
---@param component integer
---@return boolean
function ecs.owns(entity, component)
end

---Test if an entity has a role
---@param entity integer
---@param role integer
---@return boolean
function ecs.has_role(entity, role)
end

---Test whether an entity is alive
---@param entity integer
---@return boolean
function ecs.is_alive(entity)
end

---Test whether an entity is valid
---@param entity integer
---@return boolean
function ecs.is_valid(entity)
end

---Get alive identifier
---@param entity integer
---@return integer
function ecs.get_alive(entity)
end

---Ensure id is alive
---@param entity integer
function ecs.ensure(entity)
end

---Test whether an entity exists
---Similar to ecs.is_alive, but ignores entity generation count
---@param entity integer
---@return boolean
function ecs.exists(entity)
end

---Add a component, type or tag to an entity
---@overload fun(subject: integer, relation: integer, object: integer)
---@param entity integer
---@param to_add integer|ecs_type_t
function ecs.add(entity, to_add)
end

---Remove a component, type or tag from an entity
---@overload fun(subject: integer, relation: integer, object: integer)
---@overload fun(subject: integer, to_remove: ecs_type_t)
---@param entity integer
---@param to_remove integer
function ecs.remove(entity, to_remove)
end

---Clear all components
---@param entity integer
function ecs.clear(entity)
end

---Enable an entity or entity's component,
---depending on the number of arguments
---@param entity integer
---@param component integer
function ecs.enable(entity, component)
end

---Disable an entity or entity's component,
---depending on the number of arguments
---@param entity integer
---@param component integer
function ecs.disable(entity, component)
end

---Count entities that have a component, type, tag or match a filter
---@param param integer|ecs_type_t|ecs_filter_t
---@return integer
function ecs.count(param)
end

---Delete children of an entity
---@param parent integer
function ecs.delete_children(parent)
end

---Declare a type
---@param name string
---@param expr string
---@return integer entity
function ecs.type(name, expr)
end

---Get the type of an entity
---@overload fun(expr: string)
---@param entity integer
---@param from_entity boolean @return a type containing entity
---@return ecs_type_t|nil
function ecs.get_type(entity, from_entity)
end

---Get the typeid of an entity
---@param entity integer
---@return integer
function ecs.get_typeid(entity)
end

---Get parent for entity with component
---@overload fun(entity: integer)
---@param entity integer
---@param component integer
---@return integer parent
function ecs.get_parent(entity, component)
end

---Enable component for entity
---@param entity integer
---@param component integer
function ecs.enable_component(entity, component)
end

---Disable component for entity
---@param entity integer
---@param component integer
function ecs.disable_component(entity, component)
end

---Test if component is enabled for entity
---@param entity integer
---@param component integer
---@return boolean
function ecs.is_component_enabled(entity, component)
end

---Add pair to entity
---@param subject integer
---@param relation integer
---@param object integer
function ecs.add_pair(subject, relation, object)
end

---Remove pair from entity
---@param subject integer
---@param relation integer
---@param object integer
function ecs.remove_pair(subject, relation, object)
end

---Test if an entity has a trait
---@param subject integer
---@param relation integer
---@param object integer
---@return boolean
function ecs.has_pair(subject, relation, object)
end

---Set relation of pair
---@param subject integer
---@param relation integer
---@param object integer
---@param value any
---@return integer
function ecs.set_pair(subject, relation, object, value)
end

---Set object of pair
---@param subject integer
---@param relation integer
---@param object integer
---@param value any
---@return integer
function ecs.set_pair_object(subject, relation, object, value)
end

---Get relation of pair
---@param subject integer
---@param relation integer
---@param object integer
---@param value any
function ecs.get_pair(subject, relation, object, value)
end

---Get mutable relation of pair
---@param subject integer
---@param relation integer
---@param object integer
---@return any, boolean
function ecs.get_mut_pair(subject, relation, object)
end

---Get object of pair
---@param subject integer
---@param relation integer
---@param object integer
---@return any, boolean
function ecs.get_pair_object(subject, relation, object)
end

---Get mutable object of pair
---@param subject integer
---@param relation integer
---@param object integer
---@return any, boolean
function ecs.get_mut_pair_object(subject, relation, object)
end

---Create a pair
---@param predicate integer
---@param object integer
---@return integer
function ecs.pair(predicate, object)
end

---Get object from pair
---@param pair integer
---@return integer
function ecs.pair_object(pair)
end

---Add a base entity to an entity
---@param entity integer
---@param base integer
function ecs.add_instanceof(entity, base)
end

---Remove a base entity from an entity
---@param entity integer
---@param base integer
function ecs.remove_instanceof(entity, base)
end

---Add a parent entity to an entity
---@param entity integer
---@param parent integer
function ecs.add_childof(entity, parent)
end

---Remove a parent entity from an entity
---@param entity integer
---@param parent integer
function ecs.remove_childof(entity, parent)
end

---Add owned flag for component (forces ownership when instantiating)
---@param entity integer
---@param component integer
function ecs.add_owned(entity, component)
end

---Get case for switch
---@param entity integer
---@param sw integer switch
---@return integer
function ecs.get_case(entity, sw)
end

---Add switch to an entity
---@param entity integer
---@param sw integer
function ecs.add_switch(entity, sw)
end

---Remove switch from an entity
---@param entity integer
---@param sw integer
function ecs.remove_switch(entity, sw)
end

---Add case to an entity
---@param entity integer
---@param sw integer
function ecs.add_case(entity, sw)
end

---Remove case from an entity
---@param entity integer
---@param sw integer
function ecs.remove_case(entity, sw)
end

---Create a bitmask component
---@param name string name
---@param descriptor string @format: "{name = constant, ...}"
---@return integer @entity
function ecs.bitmask(name, descriptor)
end

---Create an enum component
---@param name string name
---@param descriptor string @format: "{name, ...}"
---@return integer @entity
function ecs.enum(name, descriptor)
end

---Create an array component
---@param name string name
---@param type string
---@param count integer
---@return integer @entity
function ecs.array(name, type, count)
end

---Create a struct component
---@param name string name
---@param descriptor string @format: "{type member; ...}"
---@return integer @entity
function ecs.struct(name, descriptor)
end

---Clone a component with a new name
---@param meta_type string @name
---@param alias string
---@return integer @entity
function ecs.alias(meta_type, alias)
end

---Get the value of an entity's component,
---returns nil if the entity does not have the component
---@param entity integer
---@param component integer
---@return table|nil
function ecs.get(entity, component)
end

---Get the value of an entity's component,
---adds the component if it doesn't exist,
---the second return value indicates whether it was added
---@param entity integer
---@param component integer
---@return table, boolean
function ecs.get_mut(entity, component)
end

---Patch component value without triggering OnSet systems, `copy` action.
---Returns true if component was added, false otherwise
---@param entity integer
---@param component integer
---@param value table
---@return boolean
function ecs.patch(entity, component, value)
end

---Set the value of a component,
---equivalent to ecs.get_mut() + ecs.modified(),
---it does not trigger the `copy` component action
---@param entity integer
---@param component integer
---@param v table
---@return integer entity
function ecs.set(entity, component, v)
end

---Create a new reference
---@param entity integer
---@param component integer
---@return ecs_ref_t
function ecs.ref(entity, component)
end

---Get component value from cached reference
---@overload fun(ref: ecs_ref_t)
---@param ref ecs_ref_t
---@param entity integer
---@param component integer
---@return table|nil
function ecs.get_ref(ref, entity, component)
end

---Get the value of a singleton component
---@param component integer
---@return table|nil
function ecs.singleton_get(component)
end

---Patch singleton component value, returns true if component was added
---@param component integer
---@param value table
---@return boolean
function ecs.singleton_patch(component, value)
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

---Get child count for entity
---@param entity integer
---@return integer
function ecs.get_child_count(entity)
end

---Create a scope iterator
---@overload fun(parent: integer)
---@param parent integer
---@param filter ecs_filter_t
---@return ecs_iter_t
function ecs.scope_iter(parent, filter)
end

---Progress the scope iterator
---@param it ecs_iter_t
---@return boolean
function ecs.scope_next(it)
end

---Set the current scope, returns previous scope
---@param scope integer
---@return integer
function ecs.set_scope(scope)
end

---Get the current scope
---@return integer
function ecs.get_scope()
end

---Set name prefix for newly created entities
---@param prefix string
---@return string
function ecs.set_name_prefix(prefix)
end

---Create N new entities with an optional component
---@overload fun(n: integer)
---@overload fun(n: integer, noreturn: boolean)
---@overload fun(type: integer, count: integer)
---@param type integer @optional
---@param n integer
---@param noreturn boolean @do not return the entity ID's
---@return integer[]|nil
function ecs.bulk_new(type, n, noreturn)
end

local Component = ecs.lookup("Component")

ecs.bulk_new(10)
ecs.bulk_new(Component, 10)

--Does not return the entity ID's
ecs.bulk_new(10, true)
ecs.bulk_new(Component, 10, true)

---Delete entities matching a filter
---@overload fun()
---@param filter ecs_filter_t
function ecs.bulk_delete(filter)
end

---Get column from iterator
---@param it ecs_iter_t
---@param col integer
---@return table
function ecs.column(it, col)
end

---Returns all columns, the last return value
---is the array of matched entities
---@param it ecs_iter_t
---@return any
function ecs.columns(it)
end

---Test if column is owned
---@param it ecs_iter_t
---@param column integer
function ecs.is_owned(it, column)
end

---Obtain the entity id of the signature column
---@param it ecs_iter_t
---@param column integer
---@return integer
function ecs.column_entity(it, column)
end

---Create a filter iterator
---@param filter ecs_filter_t
---@return ecs_iter_t
function ecs.filter_iter(filter)
end

---Progress the filter iterator
---@param it ecs_iter_t
---@return boolean
function ecs.fiter_next(it)
end

---Create a term iterator
---@param term integer
---@return ecs_iter_t
function ecs.term_iter(term)
end

---Progress the term iterator
---@param it ecs_iter_t
---@return boolean
function ecs.term_next(it)
end

---Create a query
---@param desc ecs_filter_t
---@return ecs_query_t
function ecs.query(desc)
end

---Create a subquery
---@param query ecs_query_t
---@param desc ecs_filter_t
---@return ecs_query_t
function ecs.subquery(query, desc)
end

---Create a query iterator
---@param query ecs_query_t
---@return ecs_iter_t
function ecs.query_iter(query)
end

---Progress the query iterator
---@overload fun(it: ecs_iter_t)
---@param it ecs_iter_t
---@param filter ecs_filter_t @optional
---@return boolean
function ecs.query_next(it, filter)
end

---Check whether the query data changed since last iteration
---@param query ecs_query_t
---@return boolean
function ecs.query_changed(query)
end

---Create generic for loop iterator for a query/iterator
---Jumping out of the loop will leave the last iteration's
---components unmodified.
---@overload fun(it: ecs_iter_t)
---@param query ecs_query_t
function ecs.each(query)
end

---Create a system
---@param callback fun(it: ecs_iter_t)
---@param name string
---@param phase integer
---@param signature string @optional
---@return integer @entity
function ecs.system(callback, name, phase, signature)
end

---Create a trigger for a single component
---@param callback fun(it: ecs_iter_t)
---@param name string
---@param events integer|integer[]
---@param desc string|ecs_term_t @expression or term
---@return integer @entity
function ecs.trigger(callback, name, events, desc)
end

---Create an observer
---@param callback fun(it: ecs_iter_t)
---@param name string
---@param events integer|integer[]
---@param filter ecs_filter_t
---@return integer @entity
function ecs.observer(callback, name, events, filter)
end

---Run a specific system manually
---@overload fun(system: integer, delta_time: number)
---@param system integer
---@param delta_time number
---@param param any @optional
---@return integer
function ecs.run(system, delta_time, param)
end

---Set system context, sets ecs_iter_t.param
---@param system integer
---@param param any
---@return integer
function ecs.set_system_context(system, param)
end

---Create a snapshot
---@overload fun()
---@param it ecs_iter_t
---@return ecs_snapshot_t
function ecs.snapshot(it)
end

---Restore and collect snapshot
---@param snapshot ecs_snapshot_t
function ecs.snapshot_restore(snapshot)
end

---Create a snapshot iterator
---@param snapshot ecs_snapshot_t
---@param filter ecs_filter_t
---@return ecs_iter_t
function ecs.snapshot_iter(snapshot, filter)
end

---Progress snapshot iterator
---@param it ecs_iter_t
---@return boolean
function ecs.snapshot_next(it)
end

---Create a module, export named entities
---to the optional export table
---@param name string
---@param export table @optional
---@param cb function @import callback
---@return integer @entity
function ecs.module(name, export, cb)
end

---EXPERIMENTAL: Import a loaded module's named entities
---@overload fun(name: string)
---@param name string
---@param t any table @optional
---@return table|nil
function ecs.import(name, t)
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

---Enable or disable tracing with colors
---@param enable boolean
function ecs.tracing_color_enable(enable)
end

---Similar to the standard assert(), but the assertion
---will always succeed when built with -DNDEBUG
---@param v any
---@param message any
function ecs.assert(v, message)
end

---Get the size and alignment of a component
---@param component integer
---@return integer, integer
function ecs.sizeof(component)
end

---Get primitive type
---@param component integer
---@return integer|nil
function ecs.is_primitive(component)
end

---Preallocate a table with array/record elements
---@param narr integer optional
---@param nrec integer optional
---@return table
function ecs.createtable(narr, nrec)
end

---Set a component to be zero-initialized
---@param component integer
function ecs.zero_init(component)
end

---Get world pointer
---@return lightuserdata
function ecs.world_ptr()
end

---Get the constants from an enum or bitmask component
---@param type integer
---@param out table @optional table to merge with
---@param prefix string @prefix to omit from names
---@param flags string @convert to lowercase ("l")
---@return table
function ecs.meta_constants(type, out, prefix, flags)
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

---Set timer timeout
---@param timer integer
---@param timeout integer
---@return integer
function ecs.set_timeout(timer, timeout)
end

---Get timer timeout
---@param timer integer
---@return number
function ecs.get_timeout(timer)
end

---Set timer interval
---@param timer integer
---@param interval integer
---@return integer
function ecs.set_interval(timer, interval)
end

---Get timer interval
---@param timer integer
---@return number
function ecs.get_interval(timer)
end

---Start timer
---@param timer integer
function ecs.start_timer(timer)
end

---Stop timer
---@param timer integer
function ecs.stop_timer(timer)
end

---Set rate filter
---@param filter integer
---@param rate integer
---@param source integer
---@return integer
function ecs.set_rate_filter(filter, rate, source)
end

---Assign tick source to system
---@param system integer
---@param tick_source integer
function ecs.set_tick_source(system, tick_source)
end

---Create a new pipeline
---@param name string
---@param expr string
---@return integer
function ecs.pipeline(name, expr)
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

---Get the number of threads
---DEPRECATED: use ecs.get_stage_count()
---@return integer
function ecs.get_threads()
end

---Get current thread index
---DEPRECATED: use ecs.get_stage_id()
---@return integer
function ecs.get_thread_index()
end

---Get the number of stages
---@return integer
function ecs.get_stage_count()
end

---Get current stage id
---@return integer
function ecs.get_stage_id()
end

---EXPERIMENTAL: Create a new world,
---returns a new set of API functions tied to the world
---@return table
function ecs.init()
end

---EXPERIMENTAL: Destroy world,
---invalid for the default world
function ecs.fini()
end

---@class ecs_world_info_t
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
---@field frame_count_total integer
---@field merge_count_total integer
---@field pipeline_build_count_total integer
---@field systems_ran_frame integer
local ecs_world_info_t = {}

---@class EcsLuaGauge
---@field avg number[60]
---@field min number[60]
---@field max number[60]
local EcsLuaGauge = {}

---@class EcsLuaCounter
---@field rate EcsLuaGauge
---@field value number[60]
local EcsLuaCounter = {}

---@class EcsLuaWorldStats
---@field dummy_ integer
---@field entity_count EcsLuaGauge
---@field component_count EcsLuaGauge
---@field query_count EcsLuaGauge
---@field system_count EcsLuaGauge
---@field table_count EcsLuaGauge
---@field empty_table_count EcsLuaGauge
---@field singleton_table_count EcsLuaGauge
---@field matched_entity_count EcsLuaGauge
---@field matched_table_count EcsLuaGauge
---@field new_count EcsLuaCounter
---@field bulk_new_count EcsLuaCounter
---@field delete_count EcsLuaCounter
---@field clear_count EcsLuaCounter
---@field add_count EcsLuaCounter
---@field remove_count EcsLuaCounter
---@field set_count EcsLuaCounter
---@field discard_count EcsLuaCounter
---@field world_time_total_raw EcsLuaCounter
---@field world_time_total EcsLuaCounter
---@field frame_time_total EcsLuaCounter
---@field system_time_total EcsLuaCounter
---@field merge_time_total EcsLuaCounter
---@field fps EcsLuaGauge
---@field delta_time EcsLuaGauge
---@field frame_count_total EcsLuaCounter
---@field merge_count_total EcsLuaCounter
---@field pipeline_build_count_total EcsLuaCounter
---@field systems_ran_frame EcsLuaCounter
---@field t integer
local EcsLuaWorldStats = {}

---Get world info
---@return ecs_world_info_t
function ecs.world_info()
end

---Get world stats
---@return EcsLuaWorldStats
function ecs.world_stats()
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

---@alias ecs_emmyopt
---| '"t"'   # Handle member struct's type as table

---Create an EmmyLua class annotation
---@overload fun(type: integer)
---@param type integer
---@param options ecs_emmyopt
---@return string
function ecs.emmy_class(type, options)
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

local dynamic = 1

--Builtin components
ecs.Component = 1
ecs.ComponentLifecycle = 2
ecs.Type = 3
ecs.Trigger = 6
ecs.Query = 7
ecs.System = 10
ecs.TickSource = 11
ecs.PipelineQuery = 12
ecs.Timer = 13
ecs.RateFilter = 14
ecs.Primitive = dynamic
ecs.Enum = dynamic
ecs.Bitmask = dynamic
ecs.Member = dynamic
ecs.Struct = dynamic
ecs.Array = dynamic
ecs.Vector = dynamic
ecs.Map = dynamic
ecs.MetaType = dynamic
ecs.MetaTypeSerializer = dynamic
ecs.LuaGauge = dynamic
ecs.LuaCounter = dynamic
ecs.LuaWorldStats = dynamic
ecs.type_op_kind_t = dynamic
ecs.type_op_t = dynamic

--Builtin entities
ecs.Flecs = 257
ecs.FlecsCore = 258
ecs.World = 256
ecs.Wildcard = 266
ecs.This = 267
ecs.Transitive = 268
ecs.Final = 269
ecs.Tag = 270
ecs.Name = 271
ecs.Symbol = 272
ecs.ChildOf = 276
ecs.IsA = 277
ecs.Module = 259
ecs.Prefab = 260
ecs.Disabled = 261
ecs.Hidden = 262
ecs.OnAdd = 286
ecs.OnRemove = 287
ecs.OnSet = 288
ecs.UnSet = 289
ecs.OnDelete = 290
ecs.OnDeleteObject = 299
ecs.Remove = 306
ecs.Delete = 307
ecs.Throw = 308
ecs.OnDemand = 316
ecs.Monitor = 317
ecs.DisabledIntern = 318
ecs.Inactive = 319
ecs.Pipeline = 320
ecs.PreFrame = 321
ecs.OnLoad = 322
ecs.PostLoad = 323
ecs.PreUpdate = 324
ecs.OnUpdate = 325
ecs.OnValidate = 326
ecs.PostUpdate = 327
ecs.PreStore = 328
ecs.OnStore = 329
ecs.PostFrame = 330

--Enum values
ecs.PrimitiveType = 0
ecs.BitmaskType = 1
ecs.EnumType = 2
ecs.StructType = 3
ecs.ArrayType = 4
ecs.VectorType = 5
ecs.MapType = 6
ecs.Bool = 1
ecs.Char = 2
ecs.Byte = 3
ecs.U8 = 4
ecs.U16 = 5
ecs.U32 = 6
ecs.U64 = 7
ecs.I8 = 8
ecs.I16 = 9
ecs.I32 = 10
ecs.I64 = 11
ecs.F32 = 12
ecs.F64 = 13
ecs.UPtr = 14
ecs.IPtr = 15
ecs.String = 16
ecs.Entity = 17
ecs.OpHeader = 0
ecs.OpPrimitive = 1
ecs.OpEnum = 2
ecs.OpBitmask = 3
ecs.OpPush = 4
ecs.OpPop = 5
ecs.OpArray = 6
ecs.OpVector = 7
ecs.OpMap = 8
ecs.DefaultSet = 0
ecs.Self = 1
ecs.SuperSet = 2
ecs.SubSet = 4
ecs.Cascade = 8
ecs.All = 16
ecs.Nothing = 32
ecs.InOutDefault = 0
ecs.InOut = 1
ecs.In = 2
ecs.Out = 3
ecs.And = 0
ecs.Or = 1
ecs.Not = 2
ecs.Optional = 3
ecs.AndFrom = 4
ecs.OrFrom = 5
ecs.NotFrom = 6

--Macros
ecs.AND = 0xF900000000000000
ecs.OR = 0xF800000000000000
ecs.XOR = 0xF700000000000000
ecs.NOT = 0xF600000000000000
ecs.CASE = 0xFC00000000000000
ecs.SWITCH = 0xFB00000000000000
ecs.PAIR = 0xFA00000000000000
ecs.OWNED = 0xF500000000000000
ecs.DISABLED = 0xF400000000000000

return ecs