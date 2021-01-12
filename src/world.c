#include "private.h"

int world_new(lua_State *L)
{
    ecs_world_t *w = ecs_init();

    lua_pushcfunction(L, luaopen_ecs);
    lua_pushlightuserdata(L, w);

    lua_call(L, 1, 1);

    return 1;
}

int world_fini(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
    ecs_world_t *main = ecs_lua_get_world(L);

    if(w == main) return luaL_argerror(L, 0, "cannot destroy default world");

    ecs_fini(w);

    return 0;
}

int world_info(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
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

int world_stats(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
#if 0
    ecs_entity_t e = ecs_lookup_fullpath(w, "flecs.lua.LuaWorldInfo");
    ecs_assert(e, ECS_INTERNAL_ERROR, NULL);

    ecs_world_stats_t ws;
    ecs_get_world_stats(w, &ws);

    EcsLuaWorldStats world_stats =
    {
        .entity_count = ws.entity_count,
        .component_count = ws.component_count,
        .query_count = ws.query_count,
        .system_count = ws.system_count,
        .table_count = ws.table_count,
        .empty_table_count = ws.empty_table_count,
        .singleton_table_count = ws.singleton_table_count,
        .max_entities_per_table = ws.max_entities_per_table,
        .max_components_per_table = ws.max_components_per_table,
        .max_columns_per_table = ws.max_columns_per_table,
        .max_matched_queries_per_table = ws.max_matched_queries_per_table,
        .new_count = ws.new_count,
        .bulk_new_count = ws.bulk_new_count,
        .delete_count = ws.delete_count,
        .clear_count = ws.clear_count,
        .add_count = ws.add_count,
        .remove_count = ws.remove_count,
        .set_count = ws.set_count,
        .discard_count = ws.discard_count,
    };

    ecs_ptr_to_lua(w, L, e, &world_stats);
#endif
    lua_pushnil(L);

    return 1;
}

int dim(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    lua_Integer count = luaL_checkinteger(L, 1);

    ecs_dim(w, count);

    return 0;
}

int dim_type(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    lua_Integer count = luaL_checkinteger(L, 1);
    ecs_type_t type = checktype(L, 2);

    ecs_dim_type(w, type, count);

    return 0;
}