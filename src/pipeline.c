#include "private.h"

int new_pipeline(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 2);
    const char *expr = luaL_checkstring(L, 2);

    ecs_entity_t e = ecs_type_init(w, &(ecs_type_desc_t)
    {
        .entity = { .name = name, .add = {EcsPipeline} },
        .ids_expr = expr
    });

    ecs_set_name(w, e, name);

    lua_pushinteger(L, e);

    return 1;
}

int set_pipeline(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t pipeline = luaL_checkinteger(L, 1);

    ecs_set_pipeline(w, pipeline);

    return 0;
}

int get_pipeline(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t pipeline = ecs_get_pipeline(w);

    lua_pushinteger(L, pipeline);

    return 1;
}

int set_target_fps(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    lua_Number fps = luaL_checknumber(L, 1);

    ecs_set_target_fps(w, fps);

    return 0;
}

int progress(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    lua_Number delta_time = luaL_checknumber(L, 1);

    int b = ecs_progress(w, delta_time);

    lua_pushboolean(L, b);

    return 1;
}

int progress_cb(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L, w);

    luaL_checktype(L, 1, LUA_TFUNCTION);
    ctx->progress_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}

int set_time_scale(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    lua_Number scale = luaL_checknumber(L, 1);

    ecs_set_time_scale(w, scale);

    return 0;
}

int reset_clock(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_reset_clock(w);

    return 0;
}

int lquit(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_quit(w);

    return 0;
}

int deactivate_systems(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_deactivate_systems(w);

    return 0;
}

int set_threads(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    lua_Integer threads = luaL_checkinteger(L, 1);

    ecs_set_threads(w, threads);

    return 0;
}

int get_threads(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    int32_t threads = ecs_get_threads(w);

    lua_pushinteger(L, threads);

    return 1;
}

int get_thread_index(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    int32_t index = ecs_get_stage_id(w);

    lua_pushinteger(L, index);

    return 1;
}