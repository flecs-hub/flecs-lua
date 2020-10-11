#include "private.h"


int set_target_fps(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    lua_Number fps = luaL_checknumber(L, 1);

    ecs_set_target_fps(w, fps);

    return 0;
}

int progress(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    lua_Number delta_time = luaL_checknumber(L, 1);

    int b = ecs_progress(w, delta_time);

    lua_pushboolean(L, b); 

    return 1;
}

int progress_cb(lua_State *L)
{
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);
    ecs_world_t *w = ctx->world;

    luaL_checktype(L, 1, LUA_TFUNCTION);
    ctx->progress_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}

int world_info(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);
    const ecs_world_info_t *wi = ecs_get_world_info(w);

    ecs_entity_t e = ecs_lookup_fullpath(w, "flecs.lua.LuaWorldInfo");
    ecs_assert(e, ECS_INTERNAL_ERROR, NULL);

    struct EcsLuaWorldInfo world_info =
    {
        .last_component_id = wi->last_component_id,
        .last_id = wi->last_id,
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
        .sleep_err = wi->sleep_err,
        .frame_count_total = wi->frame_count_total,
        .merge_count_total = wi->merge_count_total,
        .pipeline_build_count_total = wi->pipeline_build_count_total,
        .systems_ran_frame = wi->systems_ran_frame,
    };

    ecs_ptr_to_lua(w, L, e, &world_info);

    return 1;
}

int lquit(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_quit(w);

    return 0;
}