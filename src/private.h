#ifndef ECS_LUA__PRIVATE_H
#define ECS_LUA__PRIVATE_H

#include <flecs_lua.h>

#include <lualib.h>
#include <lauxlib.h>

const int ecs_lua__ser;
#define ECS_LUA_SERIALIZER (&ecs_lua__ser)

const int ecs_lua__cursors;
#define ECS_LUA_CURSORS (&ecs_lua__cursors)

const int ecs_lua__types;
#define ECS_LUA_TYPES (&ecs_lua__types)

//Internal, faster version of ecs_lua_get_world() for API functions
#define ecs_lua_world(L) lua_touserdata(L, lua_upvalueindex(1))

#define ecs_lua__prolog(L) int ecs_lua__stackguard = lua_gettop(L)
#define ecs_lua__epilog(L) ecs_assert(ecs_lua__stackguard == lua_gettop(L), ECS_INTERNAL_ERROR, NULL)

#ifdef NDEBUG
    #define ecs_lua_dbg(fmt, ...)
#else
    #define ecs_lua_dbg(fmt, ...) //ecs_os_dbg(fmt, __VA_ARGS__)
#endif

/* ecs */
ecs_lua_ctx *ecs_lua_get_context(lua_State *L);

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

typedef struct ecs_lua_ctx
{
    lua_State *L;
    int flags;

    int internal;
    int error;
    int progress_ref;
    ecs_world_t *world;

    ecs_entity_t serializer_id;
    ecs_entity_t metatype_id;
}ecs_lua_ctx;

typedef struct ecs_lua_system
{
    lua_State *L;
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