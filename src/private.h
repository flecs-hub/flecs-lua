#ifndef ECS_LUA__PRIVATE_H
#define ECS_LUA__PRIVATE_H

#include <flecs_lua.h>

#include <lualib.h>
#include <lauxlib.h>

/* A 64-bit lua_Number is also recommended */
#if (lua_Unsigned)-1 != UINT64_MAX
    #error "flecs-lua requires a 64-bit lua_Integer"
#endif

#undef ECS_META_IMPL
#ifndef FLECS_LUA_IMPL
    #define ECS_META_IMPL EXTERN
#endif

extern ECS_COMPONENT_DECLARE(EcsLuaHost);

#define ECS_LUA_CONTEXT    (1)
#define ECS_LUA_CURSORS    (2)
#define ECS_LUA_TYPES      (3)
#define ECS_LUA_COLLECT    (4)
#define ECS_LUA_REGISTRY   (5)
#define ECS_LUA_APIWORLD   (6)

/* For internal API functions */
static inline ecs_world_t *ecs_lua_world_internal(lua_State *L)
{
    ecs_world_t *w = *(ecs_world_t**)lua_touserdata(L, lua_upvalueindex(1));

    if(!w) luaL_argerror(L, 0, "world was destroyed");

    return w;
}

/* Get the associated world for the userdata at the given index */
static inline ecs_world_t *ecs_lua_object_world(lua_State *L, int idx)
{
    int type = lua_getuservalue(L, idx);

    ecs_assert(type == LUA_TUSERDATA, ECS_INTERNAL_ERROR, NULL);

    ecs_world_t *w = *(ecs_world_t**)lua_touserdata(L, -1);

    lua_pop(L, 1);

    if(!w) luaL_argerror(L, 0, "world was destroyed");

    return w;
}

/* Check world against object world at the given index */
static inline void ecs_lua_check_world(lua_State *L, const ecs_world_t *world, int idx)
{
    ecs_world_t *object_world = ecs_lua_object_world(L, 1);

    if(object_world != world) luaL_argerror(L, 1, "world mismatch");
}

static inline lua_Integer checkentity(lua_State *L, ecs_world_t *world, int arg)
{
    lua_Integer entity = luaL_checkinteger(L, arg);

    if(!ecs_is_valid(world, entity)) luaL_argerror(L, arg, "invalid entity");

    return entity;
}

#ifdef NDEBUG
    #define ecs_lua_dbg(fmt, ...)
    #define ecs_lua_dbg_pad(depth)

    #define ecs_lua__prolog(L)
    #define ecs_lua__epilog(L)
    #define ecs_lua_world(L) ecs_lua_world_internal(L)
#else
    #define ecs_lua_dbg(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
    #define ecs_lua_dbg_pad(depth) printf("%*s", depth*2, "")

    #define ecs_lua__prolog(L) int ecs_lua__stackguard = lua_gettop(L)
    #define ecs_lua__epilog(L) ecs_assert(ecs_lua__stackguard == lua_gettop(L), ECS_INTERNAL_ERROR, NULL)
    #define ecs_lua_world(L) ecs_lua_world_internal(L); lua_Debug ar = {0}; if(lua_getstack(L, 1, &ar)) lua_getinfo(L, "Sl", &ar)
#endif

/* ecs */
ecs_lua_ctx *ecs_lua_get_context(lua_State *L, const ecs_world_t *world);

/* Register object with the world to be __gc'd before ecs_fini() */
void register_collectible(lua_State *L, ecs_world_t *w, int idx);

/* Creates and returns a reference in the world's registry,
   for the object on the top of the stack (and pops the object) */
int ecs_lua_ref(lua_State *L, ecs_world_t *world);

/* Pushes onto the stack the value t[ref], where t is the registry for the given world */
int ecs_lua_rawgeti(lua_State *L, ecs_world_t *world, int ref);

/* Releases the reference ref from the registry for the given world  */
void ecs_lua_unref(lua_State *L, ecs_world_t *world, int ref);

/* meta */
bool ecs_lua_query_next(lua_State *L, int idx);
int meta_constants(lua_State *L);

/* Update iterator, usually called after ecs_lua_to_iter() + ecs_*_next() */
void ecs_lua_iter_update(lua_State *L, int idx, ecs_iter_t *it);

/* iter */
ecs_iter_t *ecs_lua__checkiter(lua_State *L, int idx);
ecs_term_t checkterm(lua_State *L, const ecs_world_t *world, int arg);

/* misc */
ecs_type_t checktype(lua_State *L, int arg);
int check_filter_desc(lua_State *L, const ecs_world_t *world, ecs_filter_desc_t *desc, int arg);
ecs_filter_t *checkfilter(lua_State *L, const ecs_world_t *world, int arg);
ecs_query_t *checkquery(lua_State *L, int arg);
int ecs_lua__readonly(lua_State *L);
void ecs_lua__assert(lua_State *L, bool condition, const char *param, const char *condition_str);

#ifdef NDEBUG
    #define ecs_lua_assert(L, condition, param)
#else
    #define ecs_lua_assert(L, condition, param) ecs_lua__assert(L, condition, NULL, #condition)
#endif

typedef struct ecs_lua_ctx
{
    lua_State *L;
    ecs_world_t *world;
    int flags;

    int internal;
    int error;
    int progress_ref;
    int prefix_ref;
}ecs_lua_ctx;

typedef enum EcsLuaCallbackType
{
    EcsLuaSystem = 0,
    EcsLuaObserver
}EcsLuaCallbackType;

typedef struct ecs_lua_callback
{
    int func_ref;
    int param_ref;

    EcsLuaCallbackType type;
    const char *type_name;
}ecs_lua_callback;

typedef struct EcsLuaIter
{
    int ptr_only;
    ecs_iter_t *it;
    ecs_iter_t storage;
}EcsLuaIter;

ECS_STRUCT(EcsLuaWorldInfo,
{
    ecs_entity_t last_component_id;
    ecs_entity_t last_id;
    ecs_entity_t min_id;
    ecs_entity_t max_id;

    float delta_time_raw; //float_t
    float delta_time; //float_t
    float time_scale; //float_t
    float target_fps; //float_t
    float frame_time_total; //float_t
    float system_time_total;
    float merge_time_total;
    float world_time_total; //float_t
    float world_time_total_raw; //float_t

    int32_t frame_count_total;
    int32_t merge_count_total;
    int32_t pipeline_build_count_total;
    int32_t systems_ran_frame;
});

#define ECS_LUA_STAT_WINDOW 60

/* These two are padded to the same size as the ecs_metric_t union */
ECS_STRUCT(EcsLuaGauge,
{
    float avg[60];
    float min[60];
    float max[60];

//ECS_PRIVATE
    double pad[60];
});

ECS_STRUCT(EcsLuaGauge_,
{
    float avg[60];
    float min[60];
    float max[60];
});

ECS_STRUCT(EcsLuaCounter,
{
    EcsLuaGauge_ rate;
    double value[60];
});

ECS_STRUCT(EcsLuaWorldStats,
{
    int64_t first_;

    EcsLuaGauge entity_count;
    EcsLuaGauge entity_not_alive_count;

    EcsLuaGauge id_count;
    EcsLuaGauge tag_id_count;
    EcsLuaGauge component_id_count;
    EcsLuaGauge pair_id_count;
    EcsLuaGauge wildcard_id_count;
    EcsLuaGauge type_count;
    EcsLuaCounter id_create_count;
    EcsLuaCounter id_delete_count;

    EcsLuaGauge table_count;
    EcsLuaGauge empty_table_count;
    EcsLuaGauge tag_table_count;
    EcsLuaGauge trivial_table_count;
    EcsLuaGauge table_record_count;
    EcsLuaGauge table_storage_count;
    EcsLuaCounter table_create_count;
    EcsLuaCounter table_delete_count;

    EcsLuaGauge query_count;
    EcsLuaGauge observer_count;
    EcsLuaGauge system_count;

    EcsLuaCounter add_count;
    EcsLuaCounter remove_count;
    EcsLuaCounter delete_count;
    EcsLuaCounter clear_count;
    EcsLuaCounter set_count;
    EcsLuaCounter get_mut_count;
    EcsLuaCounter modified_count;
    EcsLuaCounter other_count;
    EcsLuaCounter discard_count;
    EcsLuaCounter batched_entity_count;
    EcsLuaCounter batched_count;

    EcsLuaCounter frame_count;
    EcsLuaCounter merge_count;
    EcsLuaCounter rematch_count;
    EcsLuaCounter pipeline_build_count;
    EcsLuaCounter systems_ran;
    EcsLuaCounter observers_ran;
    EcsLuaCounter event_emit_count;

    EcsLuaCounter world_time_raw;
    EcsLuaCounter world_time;
    EcsLuaCounter frame_time;
    EcsLuaCounter system_time;
    EcsLuaCounter emit_time;
    EcsLuaCounter merge_time;
    EcsLuaCounter rematch_time;
    EcsLuaGauge fps;
    EcsLuaGauge delta_time;

    EcsLuaCounter alloc_count;
    EcsLuaCounter realloc_count;
    EcsLuaCounter free_count;
    EcsLuaCounter outstanding_alloc_count;

    EcsLuaCounter block_alloc_count;
    EcsLuaCounter block_free_count;
    EcsLuaCounter block_outstanding_alloc_count;
    EcsLuaCounter stack_alloc_count;
    EcsLuaCounter stack_free_count;
    EcsLuaCounter stack_outstanding_alloc_count;

    EcsLuaCounter rest_request_count;
    EcsLuaCounter rest_entity_count;
    EcsLuaCounter rest_entity_error_count;
    EcsLuaCounter rest_query_count;
    EcsLuaCounter rest_query_error_count;
    EcsLuaCounter rest_query_name_count;
    EcsLuaCounter rest_query_name_error_count;
    EcsLuaCounter rest_query_name_from_cache_count;
    EcsLuaCounter rest_enable_count;
    EcsLuaCounter rest_enable_error_count;
    EcsLuaCounter rest_world_stats_count;
    EcsLuaCounter rest_pipeline_stats_count;
    EcsLuaCounter rest_stats_error_count;

    EcsLuaCounter http_request_received_count;
    EcsLuaCounter http_request_invalid_count;
    EcsLuaCounter http_request_handled_ok_count;
    EcsLuaCounter http_request_handled_error_count;
    EcsLuaCounter http_request_not_handled_count;
    EcsLuaCounter http_request_preflight_count;
    EcsLuaCounter http_send_ok_count;
    EcsLuaCounter http_send_error_count;
    EcsLuaCounter http_busy_count;

    int64_t last_;
//ECS_PRIVATE
    int32_t t;
});

ECS_STRUCT(EcsLuaTermID,
{
    ecs_entity_t id;
    /*const char *name;*/
    ecs_entity_t trav;
    uint32_t flags;
});

ECS_STRUCT(EcsLuaTerm,
{
    int64_t id;

    int32_t inout;
    EcsLuaTermID src;
    EcsLuaTermID first;
    EcsLuaTermID second;
    int32_t oper;
});

#define ECS_LUA_BUILTINS(XX) \
    XX(Flecs) \
    XX(FlecsCore) \
    XX(World) \
    XX(Wildcard) \
    XX(This) \
    XX(Transitive) \
    XX(Final) \
    XX(Tag) \
    XX(Name) \
    XX(Union) \
    XX(Symbol) \
    XX(ChildOf) \
\
    XX(Query) \
    XX(System) \
\
    XX(IsA) \
    XX(Module) \
    XX(Prefab) \
    XX(Disabled) \
\
    XX(OnAdd) \
    XX(OnRemove) \
\
    XX(OnSet) \
    XX(UnSet) \
    XX(OnDelete) \
    XX(Remove) \
    XX(Delete) \
    XX(Panic) \
\
    XX(Monitor) \
    XX(Empty) \
\
    XX(PreFrame) \
    XX(OnLoad) \
    XX(PostLoad) \
    XX(PreUpdate) \
    XX(OnUpdate) \
    XX(OnValidate) \
    XX(PostUpdate) \
    XX(PreStore) \
    XX(OnStore) \
    XX(PostFrame)

#define ECS_LUA_PRIMITIVES(XX) \
    XX(bool) \
    XX(char) \
    XX(byte) \
    XX(u8) \
    XX(u16) \
    XX(u32) \
    XX(u64) \
    XX(uptr) \
    XX(i8) \
    XX(i16) \
    XX(i32) \
    XX(i64) \
    XX(iptr) \
    XX(f32) \
    XX(f64) \
    XX(string) \
    XX(entity)

#define ECS_LUA_ENUMS(XX) \
    XX(PrimitiveType) \
    XX(BitmaskType) \
    XX(EnumType) \
    XX(StructType) \
    XX(ArrayType) \
    XX(VectorType) \
\
    XX(Bool) \
    XX(Char) \
    XX(Byte) \
    XX(U8) \
    XX(U16) \
    XX(U32) \
    XX(U64) \
    XX(I8) \
    XX(I16) \
    XX(I32) \
    XX(I64) \
    XX(F32) \
    XX(F64) \
    XX(UPtr) \
    XX(IPtr) \
    XX(String) \
    XX(Entity) \
\
    XX(OpPrimitive) \
    XX(OpEnum) \
    XX(OpBitmask) \
    XX(OpPush) \
    XX(OpPop) \
    XX(OpArray) \
    XX(OpVector) \
\
    XX(Self) \
    XX(Cascade) \
\
    XX(InOutDefault) \
    XX(InOut) \
    XX(In) \
    XX(Out) \
\
    XX(And) \
    XX(Or) \
    XX(Not) \
    XX(Optional) \
    XX(AndFrom) \
    XX(OrFrom) \
    XX(NotFrom)

#define ECS_LUA_MACROS(XX) \
    XX(AND) \
    XX(PAIR) \
    XX(OVERRIDE) \
    XX(TOGGLE)

#define ECS_LUA_TYPEIDS(XX) \
    XX(Component) \
    XX(Identifier) \
\
    XX(TickSource) \
\
    XX(Timer) \
    XX(MetaType) \
    XX(MetaTypeSerialized) \
    XX(RateFilter) \
    XX(Primitive) \
    XX(Enum) \
    XX(Bitmask) \
    XX(Member) \
    XX(Struct) \
    XX(Array) \
    XX(Vector)



#endif /* ECS_LUA__PRIVATE_H */