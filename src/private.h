#ifndef ECS_LUA__PRIVATE_H
#define ECS_LUA__PRIVATE_H

#include <flecs_lua.h>

#include <lualib.h>
#include <lauxlib.h>

/* A 64-bit lua_Number is also recommended */
#if (lua_Unsigned)-1 != UINT64_MAX
    #error "flecs-lua requires a 64-bit lua_Integer"
#endif

extern ECS_COMPONENT_DECLARE(EcsLuaHost);
extern ECS_COMPONENT_DECLARE(EcsLuaWorldInfo);
extern ECS_COMPONENT_DECLARE(EcsLuaGauge);
extern ECS_COMPONENT_DECLARE(EcsLuaCounter);
extern ECS_COMPONENT_DECLARE(EcsLuaWorldStats);

#define ECS_LUA_CONTEXT (1)
#define ECS_LUA_CURSORS (2)
#define ECS_LUA_TYPES   (3)
#define ECS_LUA_COLLECT (4)
#define ECS_LUA_SYSTEMS (5)
#define ECS_LUA_APIWORLD (6)

/* Internal version for API functions */
static inline ecs_world_t *ecs_lua_world(lua_State *L)
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

static inline bool ecs_lua_deferred(ecs_world_t *w)
{
    bool b = !ecs_defer_begin(w);

    ecs_defer_end(w);

    return b;
}

#ifdef NDEBUG
    #define ecs_lua_dbg(fmt, ...)

    #define ecs_lua__prolog(L)
    #define ecs_lua__epilog(L)
#else
    #define ecs_lua_dbg(fmt, ...) //ecs_os_dbg(fmt, __VA_ARGS__)

    #define ecs_lua__prolog(L) int ecs_lua__stackguard = lua_gettop(L)
    #define ecs_lua__epilog(L) ecs_assert(ecs_lua__stackguard == lua_gettop(L), ECS_INTERNAL_ERROR, NULL)
#endif

/* ecs */
ecs_lua_ctx *ecs_lua_get_context(lua_State *L, const ecs_world_t *world);

/* Register object with the world to be __gc'd before ecs_fini() */
void register_collectible(lua_State *L, ecs_world_t *w, int idx);

/* meta */
bool ecs_lua_query_next(lua_State *L, int idx);
int meta_constants(lua_State *L);

/* Update iterator, usually called after ecs_lua_to_iter() + ecs_*_next() */
void ecs_lua_iter_update(lua_State *L, int idx, ecs_iter_t *it);

/* iter */
ecs_iter_t *ecs_lua__checkiter(lua_State *L, int idx);

/* misc */
ecs_type_t checktype(lua_State *L, int arg);
ecs_filter_t checkfilter(lua_State *L, int idx);
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

typedef struct ecs_lua_system
{
    const char *name;
    int func_ref;
    int param_ref;
    bool trigger;
}ecs_lua_system;

ECS_STRUCT(EcsLuaWorldInfo,
{
    ecs_entity_t last_component_id;
    ecs_entity_t last_id;
    ecs_entity_t min_id;
    ecs_entity_t max_id;

    double delta_time_raw;
    double delta_time;
    double time_scale;
    double target_fps;
    double frame_time_total;
    double system_time_total;
    double merge_time_total;
    double world_time_total;
    double world_time_total_raw;

    int32_t frame_count_total;
    int32_t merge_count_total;
    int32_t pipeline_build_count_total;
    int32_t systems_ran_frame;
});

ECS_STRUCT(EcsLuaGauge,
{
    float avg[60];
    float min[60];
    float max[60];
});

ECS_STRUCT(EcsLuaCounter,
{
    EcsLuaGauge rate;
    float value[60];
});

ECS_STRUCT(EcsLuaWorldStats,
{
    int32_t dummy_;

    EcsLuaGauge entity_count;
    EcsLuaGauge component_count;
    EcsLuaGauge query_count;
    EcsLuaGauge system_count;
    EcsLuaGauge table_count;
    EcsLuaGauge empty_table_count;
    EcsLuaGauge singleton_table_count;
    EcsLuaGauge matched_entity_count;
    EcsLuaGauge matched_table_count;

    EcsLuaCounter new_count;
    EcsLuaCounter bulk_new_count;
    EcsLuaCounter delete_count;
    EcsLuaCounter clear_count;
    EcsLuaCounter add_count;
    EcsLuaCounter remove_count;
    EcsLuaCounter set_count;
    EcsLuaCounter discard_count;

    EcsLuaCounter world_time_total_raw;
    EcsLuaCounter world_time_total;
    EcsLuaCounter frame_time_total;
    EcsLuaCounter system_time_total;
    EcsLuaCounter merge_time_total;
    EcsLuaGauge fps;
    EcsLuaGauge delta_time;

    EcsLuaCounter frame_count_total;
    EcsLuaCounter merge_count_total;
    EcsLuaCounter pipeline_build_count_total;
    EcsLuaCounter systems_ran_frame;

    int32_t t;
});

#define ECS_LUA_BUILTINS(XX) \
    XX(World) \
    XX(Wildcard) \
    XX(Flecs) \
    XX(FlecsCore) \
\
    XX(Module) \
    XX(Prefab) \
    XX(Disabled) \
    XX(Hidden) \
\
    XX(OnAdd) \
    XX(OnRemove) \
\
    XX(OnSet) \
    XX(UnSet) \
\
    XX(OnDemand) \
    XX(Monitor) \
    XX(DisabledIntern) \
    XX(Inactive) \
    XX(Pipeline) \
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
    XX(PostFrame) \
\
    XX(Tag) \
    XX(Name) \
    XX(Symbol) \
    XX(ChildOf) \
    XX(IsA)

#define ECS_LUA_ENUMS(XX) \
    XX(MatchDefault) \
    XX(MatchAll) \
    XX(MatchAny) \
    XX(MatchExact) \
\
    XX(PrimitiveType) \
    XX(BitmaskType) \
    XX(EnumType) \
    XX(StructType) \
    XX(ArrayType) \
    XX(VectorType) \
    XX(MapType) \
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
    XX(OpHeader) \
    XX(OpPrimitive) \
    XX(OpEnum) \
    XX(OpBitmask) \
    XX(OpPush) \
    XX(OpPop) \
    XX(OpArray) \
    XX(OpVector) \
    XX(OpMap) \
\
    XX(DefaultSet) \
    XX(Self) \
    XX(SuperSet) \
    XX(SubSet) \
    XX(Cascade) \
    XX(All) \
    XX(Nothing)

#define ECS_LUA_MACROS(XX) \
    XX(AND) \
    XX(OR) \
    XX(XOR) \
    XX(NOT) \
    XX(CASE) \
    XX(SWITCH) \
    XX(OWNED)

#define ECS_LUA_TYPEIDS(XX) \
    XX(Component) \
    XX(ComponentLifecycle) \
    XX(Type) \
\
    XX(Trigger) \
    XX(Query) \
    XX(System) \
    XX(TickSource) \
\
    XX(PipelineQuery) \
\
    XX(Timer) \
    XX(RateFilter) \
    XX(Primitive) \
    XX(Enum) \
    XX(Bitmask) \
    XX(Member) \
    XX(Struct) \
    XX(Array) \
    XX(Vector) \
    XX(Map) \
    XX(MetaType) \
    XX(MetaTypeSerializer) \
\
    XX(LuaGauge) \
    XX(LuaCounter) \
    XX(LuaWorldStats)



#endif /* ECS_LUA__PRIVATE_H */