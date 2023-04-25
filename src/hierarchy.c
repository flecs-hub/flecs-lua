#include "private.h"

int get_child_count(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    ecs_iter_t it = ecs_term_iter(w, &(ecs_term_t){ ecs_pair(EcsChildOf, e) });

    int32_t count = 0;

    while (ecs_term_next(&it))
    {
        count += it.count;
    }

    lua_pushinteger(L, count);

    return 1;
}

int set_scope(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t scope = luaL_checkinteger(L, 1);

    ecs_entity_t prev = ecs_set_scope(w, scope);

    lua_pushinteger(L, prev);

    return 1;
}

int get_scope(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = ecs_get_scope(w);

    lua_pushinteger(L, e);

    return 1;
}

int set_name_prefix(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L, w);

    const char *prefix = NULL;

    if(!lua_isnil(L, 1)) prefix = luaL_checkstring(L, 1);

    const char *prev = ecs_set_name_prefix(w, prefix);

    lua_pushstring(L, prev);

    if(ctx->prefix_ref != LUA_NOREF)
    {
        ecs_lua_unref(L, w, ctx->prefix_ref);
        ctx->prefix_ref = LUA_NOREF;
    }

    lua_pushvalue(L, 1);
    ctx->prefix_ref = ecs_lua_ref(L, w);

    return 1;
}