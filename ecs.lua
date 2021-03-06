--EmmyLua annotations and documentation for flecs-lua
--NOTE: ecs is a native module, this is not used at runtime.

local ecs = {}

---@class ecs_type_t
local ecs_type_t = {}

---@class ecs_ref_t
local ecs_ref_t = {}

---@class ecs_filter_t
---@field include ecs_type_t
---@field exclude ecs_type_t
---@field include_kind integer @ecs.Match*
---@field exclude_kind integer @ecs.Match*
local ecs_filter_t = {}

---@class ecs_query_t
local ecs_query_t = {}

---@class ecs_snapshot_t
local ecs_snapshot_t = {}

---@class ecs_iter_t
---@field count integer
---@field columns table[]
---@field entities integer[]
---@field system integer
---@field table_count integer
---@field delta_time number
---@field delta_system_time number
---@field world_time number
---@field interrupted_by integer
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
---@param entity string|integer|integer[]
function ecs.delete(entity)
end

---Create a tag
---@param name string
---@return integer @entity
function ecs.tag(name)
end

---Get the name of an entity
---@param e integer
---@return string|nil @entity name
function ecs.name(e)
end

---Set the name of an entity
---@param e integer
---@param name string
---@return integer @entity id
function ecs.set_name(e, name)
end

---Get the symbol name of an entity
---@param e integer
---@return string|nil @entity symbol name
function ecs.symbol(e)
end

---Get the full path for an entity
---@param e integer
---@return string|nil
function ecs.fullpath(e)
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
---@param e integer @entity
---@param to_check integer|ecs_type_t
---@return boolean
function ecs.has(e, to_check)
end

---Test if an entity owns a component
---@param entity integer
---@param component integer
---@return boolean
function ecs.owns(entity, component)
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
---@param to_add integer|ecs_type_t
function ecs.add(e, to_add)
end

---Remove a component, type or tag from an entity
---@param e integer @entity
---@param to_remove integer|ecs_type_t
function ecs.remove(e, to_remove)
end

---Clear all components
---@param e integer entity
function ecs.clear(e)
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
---@param parent integer @entity
function ecs.delete_children(parent)
end

---Declare a type
---@param name string
---@param expr string
---@return integer entity
function ecs.type(name, expr)
end

---Get the type of an entity
---@param e integer @entity
---@param from_entity boolean @return a type containing e
---@return ecs_type_t|nil
function ecs.get_type(e, from_entity)
end

---Get the typeid of an entity
---@param entity integer
---@return integer
function ecs.get_typeid(entity)
end

---Get parent for entity with component
---@param e integer entity
---@param c integer optional component
---@return integer parent
function ecs.get_parent(e, c)
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

---Add a trait to an entity
---@param e integer
---@param component integer
---@param trait integer
function ecs.add_trait(e, component, trait)
end

---Remove a trait from an entity
---@param e integer
---@param component integer
---@param trait integer
function ecs.remove_trait(e, component, trait)
end

---Test if an entity has a trait
---@param e integer
---@param component integer
---@param trait integer
---@return boolean
function ecs.has_trait(e, component, trait)
end

---Set trait for component
---@param e integer
---@param component integer
---@param trait integer
---@param v table
function ecs.set_trait(e, component, trait, v)
end

---Set tag trait for component,
---this can only be used with traits that are not components
---@param e integer
---@param component integer
---@param trait integer
---@param v table
function ecs.set_trait_tag(e, trait, component, v)
end

---Get trait for component
---@param e integer
---@param component integer
---@param trait integer
---@return any
function ecs.get_trait(e, component, trait)
end

---Get trait tag for component
---@param e integer
---@param component integer
---@param trait integer
---@return any
function ecs.get_trait_tag(e, trait, component)
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
---@param ref ecs_ref_t
---@param entity integer @optional
---@param component integer @optional
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
---@param parent integer
---@param filter ecs_filter_t @optional
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
---@param type integer|string @optional
---@param n integer
---@param noreturn boolean @do not return the entity ID's
---@return integer[]
function ecs.bulk_new(type, n, noreturn)
end

local Component = ecs.lookup("Component")

ecs.bulk_new(10)
ecs.bulk_new(Component, 10)
ecs.bulk_new("Component", 10)

--Does not return the entity ID's
ecs.bulk_new(10, true)
ecs.bulk_new(Component, 10, true)

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

---Create a query
---@param sig string
---@return ecs_query_t
function ecs.query(sig)
end

---Create a subquery
---@param query ecs_query_t
---@param sig string
---@return ecs_query_t
function ecs.subquery(query, sig)
end

---Create a query iterator
---@param query ecs_query_t
---@return ecs_iter_t
function ecs.query_iter(query)
end

---Progress the query iterator
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
---@param q_it ecs_query_t|ecs_iter_t
function ecs.each(q_it)
end

---Create a system
---@param func function
---@param name string
---@param phase integer
---@param signature string @optional
---@return integer @entity
function ecs.system(func, name, phase, signature)
end

---Create a trigger for a single component
---@param func function
---@param name string
---@param kind integer @OnAdd or OnRemove
---@param component string
---@return integer @entity
function ecs.trigger(func, name, kind, component)
end

---Run a specific system manually
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
---@param it ecs_iter_t @optional
---@param next_action function @optional
---@return ecs_snapshot_t
function ecs.snapshot(it, next_action)
end

---Restore a snapshot
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

---Get current number of threads
---@return integer
function ecs.get_threads()
end

---Get current thread index
---@return integer
function ecs.get_thread_index()
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
---@field sleep_err number
---@field frame_count_total integer
---@field merge_count_total integer
---@field pipeline_build_count_total integer
---@field systems_ran_frame integer
local ecs_world_info_t = {}

---@class ecs_world_stats_t
---@field entity_count integer
---@field component_count integer
---@field query_count integer
---@field system_count integer
---@field table_count integer
---@field empty_table_count integer
---@field singleton_table_count integer
---@field max_entities_per_table integer
---@field max_components_per_table integer
---@field max_columns_per_table integer
---@field max_matched_queries_per_table integer
---@field new_count integer
---@field bulk_new_count integer
---@field delete_count integer
---@field clear_count integer
---@field add_count integer
---@field remove_count integer
---@field set_count integer
---@field discard_count integer
local ecs_world_stats_t = {}

---Get world info
---@return ecs_world_info_t
function ecs.world_info()
end

---Get world stats
---@return ecs_world_stats_t
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

local undef = 111

ecs.Component = 1
ecs.ComponentLifecycle = 2
ecs.Type = 3
ecs.Name = 6
ecs.Trigger = 4
ecs.System = 5
ecs.TickSource = 7
ecs.SignatureExpr = 8
ecs.Signature = 9
ecs.Query = 10
ecs.IterAction = 11
ecs.Context = 12
ecs.PipelineQuery = 13
ecs.Timer = 14
ecs.RateFilter = 15
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
ecs.PrimitiveType = 0
ecs.BitmaskType = 1
ecs.EnumType = 2
ecs.StructType = 3
ecs.ArrayType = 4
ecs.VectorType = 5
ecs.MapType = 6
ecs.Primitive = undef
ecs.Enum = undef
ecs.Bitmask = undef
ecs.Member = undef
ecs.Struct = undef
ecs.Array = undef
ecs.Vector = undef
ecs.Map = undef
ecs.MetaType = undef
ecs.ecs_type_op_kind_t = undef
ecs.ecs_type_op_t = undef
ecs.MetaTypeSerializer = undef
ecs.Bool = 0
ecs.Char = 1
ecs.Byte = 2
ecs.U8 = 3
ecs.U16 = 4
ecs.U32 = 5
ecs.U64 = 6
ecs.I8 = 7
ecs.I16 = 8
ecs.I32 = 9
ecs.I64 = 10
ecs.F32 = 11
ecs.F64 = 12
ecs.UPtr = 13
ecs.IPtr = 14
ecs.String = 15
ecs.Entity = 16
ecs.OpHeader = 0
ecs.OpPrimitive = 1
ecs.OpEnum = 2
ecs.OpBitmask = 3
ecs.OpPush = 4
ecs.OpPop = 5
ecs.OpArray = 6
ecs.OpVector = 7
ecs.OpMap = 8
ecs.INSTANCEOF = 0xFE00000000000000
ecs.CHILDOF = 0xFD00000000000000
ecs.TRAIT = 0xFA00000000000000
ecs.AND = 0xF900000000000000
ecs.OR = 0xF800000000000000
ecs.XOR = 0xF700000000000000
ecs.NOT = 0xF600000000000000
ecs.CASE = 0xFC00000000000000
ecs.SWITCH = 0xFB00000000000000
ecs.OWNED = 0xF500000000000000

return ecs