#include "private.h"

int world_new(lua_State *L)
{
    ecs_world_t *wdefault = ecs_lua_get_world(L);
    ecs_assert(wdefault != NULL, ECS_INTERNAL_ERROR, NULL);

    ecs_world_t *w2 = ecs_init();

    ECS_IMPORT(w2, FlecsLua);

    ecs_lua_set_state(w2, L);

    lua_pushcfunction(L, luaopen_ecs);
    ecs_world_t **ptr = lua_newuserdata(L, sizeof(ecs_world_t*));
    *ptr = w2;

    luaL_setmetatable(L, "ecs_world_t");

    /* The __gc of the default world will collect all other worlds */
    register_collectible(L, wdefault, -1);

    lua_call(L, 1, 1);

    return 1;
}

int world_gc(lua_State *L)
{
    ecs_world_t *wdefault = ecs_lua_get_world(L);
    ecs_world_t **ptr = lua_touserdata(L, 1);
    ecs_world_t *w = *ptr;

    if(!w) return 0;

    lua_rawgetp(L, LUA_REGISTRYINDEX, w);
    lua_rawgeti(L, -1, ECS_LUA_COLLECT);

    int idx = lua_absindex(L, -1);

    lua_pushnil(L);

    while(lua_next(L, idx))
    {
        int ret = luaL_callmeta(L, -2, "__gc");
        ecs_assert(ret != 0, ECS_INTERNAL_ERROR, NULL);

        lua_pop(L, 2); /* callmeta pushes a value */
    }

    lua_pop(L, 2);

    if(w != wdefault)
    {
        /* registry[world] = nil */
        lua_pushnil(L);
        lua_rawsetp(L, LUA_REGISTRYINDEX, w);

        ecs_fini(w);
    }

    *ptr = NULL;

    return 0;
}

int world_fini(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
    ecs_world_t *default_world = ecs_lua_get_world(L);

    if(w == default_world) return luaL_argerror(L, 0, "cannot destroy default world");

    luaL_callmeta(L, lua_upvalueindex(1), "__gc");

    return 0;
}

int world_info(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
    const ecs_world_info_t *wi = ecs_get_world_info(w);

    ecs_assert(ecs_id(EcsLuaWorldInfo) != 0, ECS_INTERNAL_ERROR, NULL);

    EcsLuaWorldInfo world_info =
    {
        .last_component_id = wi->last_component_id,
        .min_id = wi->min_id,
        .max_id = wi->max_id,
        .delta_time_raw = wi->delta_time_raw,
        .delta_time = wi->delta_time,
        .time_scale = wi->time_scale,
        .target_fps = wi->target_fps,
        .frame_time_total = wi->frame_time_total,
        .system_time_total = wi->system_time_total,
        .merge_time_total = wi->merge_time_total,
        .world_time_total = wi->world_time_total,
        .world_time_total_raw = wi->world_time_total_raw,
        .frame_count_total = wi->frame_count_total,
        .merge_count_total = wi->merge_count_total,
        .pipeline_build_count_total = wi->pipeline_build_count_total,
        .systems_ran_frame = wi->systems_ran_frame,
    };

    ecs_ptr_to_lua(w, L, ecs_id(EcsLuaWorldInfo), &world_info);

    return 1;
}

int world_stats(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_assert(ecs_id(EcsLuaWorldStats) != 0, ECS_INTERNAL_ERROR, NULL);

    //sizeof(EcsLuaWorldStats)
    //sizeof(ecs_world_stats_t)

    EcsLuaWorldStats ws;
    ecs_world_stats_get(w, (ecs_world_stats_t*)&ws);

    ecs_ptr_to_lua(w, L, ecs_id(EcsLuaWorldStats), &ws);

    return 1;
}

int dim(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    lua_Integer count = luaL_checkinteger(L, 1);

    ecs_dim(w, count);

    return 0;
}
