#ifndef ECS_LUA__PRIVATE_H
#define ECS_LUA__PRIVATE_H

#include <flecs_lua.h>

#include <lualib.h>
#include <lauxlib.h>

const int ecs_lua__ser;
#define ECS_LUA_SERIALIZER (&ecs_lua__ser)

#define ECS_LUA_CONTEXT (1)
#define ECS_LUA_CURSORS (2)
#define ECS_LUA_TYPES   (3)
#define ECS_LUA_COLLECT (4)
#define ECS_LUA_SYSTEMS (5)

/* Internal version for API functions */
static inline ecs_world_t *ecs_lua_world(lua_State *L)
{
    ecs_world_t *w = *(ecs_world_t**)lua_touserdata(L, lua_upvalueindex(1));

    if(!w) luaL_argerror(L, 0, "world was destroyed");

    return w;
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
ecs_lua_ctx *ecs_lua_get_context(lua_State *L, ecs_world_t *world);

/* Register object with the world to be __gc'd before ecs_fini() */
void register_collectible(lua_State *L, ecs_world_t *w, int idx);

/* meta */
bool ecs_lua_query_next(lua_State *L, int idx);

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

    ecs_entity_t serializer_id;
    ecs_entity_t metatype_id;
}ecs_lua_ctx;

typedef struct ecs_lua_system
{
    lua_State *L;
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
    double sleep_err;

    int32_t frame_count_total;
    int32_t merge_count_total;
    int32_t pipeline_build_count_total;
    int32_t systems_ran_frame;
});


ECS_STRUCT(EcsLuaWorldStats,
{
    int32_t entity_count;
    int32_t component_count;
    int32_t query_count;
    int32_t system_count;
    int32_t table_count;
    int32_t empty_table_count;
    int32_t singleton_table_count;
    int32_t max_entities_per_table;
    int32_t max_components_per_table;
    int32_t max_columns_per_table;
    int32_t max_matched_queries_per_table;

    int32_t new_count;
    int32_t bulk_new_count;
    int32_t delete_count;
    int32_t clear_count;
    int32_t add_count;
    int32_t remove_count;
    int32_t set_count;
    int32_t discard_count;
});

#define ECS_LUA_ENUMS(XX) \
    XX(MatchDefault) \
    XX(MatchAll) \
    XX(MatchAny) \
    XX(MatchExact) \
\
    XX(Module) \
    XX(Prefab) \
    XX(Hidden) \
    XX(Disabled) \
    XX(DisabledIntern) \
    XX(Inactive) \
    XX(OnDemand) \
    XX(Monitor) \
    XX(Pipeline) \
\
    XX(OnAdd) \
    XX(OnRemove) \
\
    XX(OnSet) \
    XX(UnSet) \
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
    XX(Flecs) \
    XX(FlecsCore) \
    XX(World) \
    XX(Singleton) \
    XX(Wildcard)

#define ECS_LUA_MACROS(XX) \
    XX(INSTANCEOF) \
    XX(CHILDOF) \
    XX(TRAIT) \
    XX(AND) \
    XX(OR) \
    XX(XOR) \
    XX(NOT) \
    XX(CASE) \
    XX(SWITCH) \
    XX(OWNED)





#endif /* ECS_LUA__PRIVATE_H */