#ifndef ECS_LUA__PRIVATE_H
#define ECS_LUA__PRIVATE_H

#include <flecs_lua.h>

#include <lualib.h>
#include <lauxlib.h>

#define ecs_lua__prolog(L) int ecs_lua__stackguard = lua_gettop(L)
#define ecs_lua__epilog(L) ecs_assert(ecs_lua__stackguard == lua_gettop(L), ECS_INTERNAL_ERROR, NULL)

ecs_lua_ctx *ecs_lua_get_context(lua_State *L);

ecs_iter_t *ecs_lua__checkiter(lua_State *L, int idx);

typedef struct ecs_lua_ctx
{
    lua_State *L;
    int flags;

    int internal;
    int error;
    int progress_ref;
    ecs_world_t *world;
}ecs_lua_ctx;

ECS_STRUCT(EcsLuaWorldInfo,
{
    ecs_entity_t last_component_id;
    ecs_entity_t last_id;
    ecs_entity_t min_id;
    ecs_entity_t max_id;

    float delta_time_raw;
    float delta_time;
    float time_scale;
    float target_fps;
    float frame_time_total;
    float system_time_total;
    float merge_time_total;
    float world_time_total;
    float world_time_total_raw;
    float sleep_err;

    int32_t frame_count_total;
    int32_t merge_count_total;
    int32_t pipeline_build_count_total;
    int32_t systems_ran_frame;
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


int new_entity(lua_State *L);
int delete_entity(lua_State *L);
int new_tag(lua_State *L);
int entity_name(lua_State *L);
int lookup_entity(lua_State *L);
int lookup_fullpath(lua_State *L);
int entity_has(lua_State *L);
int has_role(lua_State *L);
int is_alive(lua_State *L);
int exists(lua_State *L);
int add_type(lua_State *L);
int remove_type(lua_State *L);
int clear_entity(lua_State *L);

int new_array(lua_State *L);
int new_struct(lua_State *L);
int new_alias(lua_State *L);

int get_func(lua_State *L);
int get_mut(lua_State *L);

int mutable_modified(lua_State *L);
int set_func(lua_State *L);

int singleton_get(lua_State *L);
int singleton_get_mut(lua_State *L);
int mutable_modified(lua_State *L);
int singleton_set(lua_State *L);

int bulk_new(lua_State *L);

int column(lua_State *L);
int columns(lua_State *L);

int new_system(lua_State *L);
int new_module(lua_State *L);

int print_log(lua_State *L);
int print_err(lua_State *L);
int print_dbg(lua_State *L);
int print_warn(lua_State *L);
int assert_func(lua_State *L);

int get_time(lua_State *L);
int time_measure(lua_State *L);

int set_target_fps(lua_State *L);
int progress(lua_State *L);
int progress_cb(lua_State *L);
int world_info(lua_State *L);
int lquit(lua_State *L);



#endif /* ECS_LUA__PRIVATE_H */