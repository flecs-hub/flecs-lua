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

int lquit(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_quit(w);

    return 0;
}