#include "private.h"

ecs_query_t *checkquery(lua_State *L, int arg)
{
    ecs_query_t **query = luaL_checkudata(L, 1, "ecs_query_t");

    if(ecs_query_orphaned(*query)) luaL_argerror(L, 1, "parent query was collected");

    return *query;
}

int query_gc(lua_State *L)
{ecs_os_dbg("QUERY_GC"); fflush(stdout);
    ecs_query_t *query = checkquery(L, 1);

    ecs_query_free(query);

    return 0;
}

int query_new(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    const char *sig = luaL_checkstring(L, 1);
    ecs_query_t *query = ecs_query_new(w, sig);

    ecs_query_t **ptr = lua_newuserdata(L, sizeof(ecs_query_t*));
    *ptr = query;

    luaL_setmetatable(L, "ecs_query_t");

    return 1;
}

int subquery_new(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_query_t *parent = checkquery(L, 1);
    const char *sig = luaL_checkstring(L, 2);

    ecs_query_t *query = ecs_subquery_new(w, parent, sig);

    ecs_query_t **ptr = lua_newuserdata(L, sizeof(ecs_query_t*));
    *ptr = query;

    return 1;
}

int query_iter(lua_State *L)
{ecs_os_dbg("QUERY_iter");
    ecs_world_t *w = ecs_lua_get_world(L);
    ecs_query_t *query = checkquery(L, 1);

    ecs_iter_t it = ecs_query_iter(query);

    /* will push with no columns because it->count = 0 */
    ecs_iter_to_lua(&it, L, NULL, true);

    return 1;
}

int query_next(lua_State *L)
{
    int b = ecs_lua_query_next(L, 1);

    lua_pushboolean(L, b);

    return 1;
}

int query_changed(lua_State *L)
{
    ecs_query_t *query = checkquery(L, 1);

    int b = ecs_query_changed(query);

    lua_pushboolean(L, b);

    return 1;
}