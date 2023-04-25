#include "private.h"

ecs_query_t *checkquery(lua_State *L, int arg)
{
    ecs_query_t **query = luaL_checkudata(L, arg, "ecs_query_t");

    if(!*query) luaL_argerror(L, arg, "query was collected");

    if(ecs_query_orphaned(*query)) luaL_argerror(L, arg, "parent query was collected");

    return *query;
}

int query_gc(lua_State *L)
{
    ecs_query_t **ptr = luaL_checkudata(L, 1, "ecs_query_t");
    ecs_query_t *query = *ptr;

    if(query) ecs_query_fini(query);

    *ptr = NULL;

    return 0;
}

int query_new(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *sig = luaL_optstring(L, 1, NULL);

    ecs_query_desc_t desc = { .filter.expr = sig };

    if(sig == NULL) check_filter_desc(L, w, &desc.filter, 1);

    ecs_query_t *query = ecs_query_init(w, &desc);

    if(!query)
    {
        lua_pushnil(L);
        return 1;
    }

    ecs_query_t **ptr = lua_newuserdata(L, sizeof(ecs_query_t*));
    *ptr = query;

    /* Associate world with the object for sanity checks */
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_setuservalue(L, -2);

    luaL_setmetatable(L, "ecs_query_t");
    register_collectible(L, w, -1);

    return 1;
}

int subquery_new(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_query_t *parent = checkquery(L, 1);
    const char *sig = luaL_optstring(L, 2, NULL);

    ecs_query_desc_t desc =
    {
        .filter.expr = sig,
        .parent = parent
    };

    if(sig == NULL) check_filter_desc(L, w, &desc.filter, 2);

    ecs_query_t *query = ecs_query_init(w, &desc);

    ecs_query_t **ptr = lua_newuserdata(L, sizeof(ecs_query_t*));
    *ptr = query;

    return 1;
}

int query_iter(lua_State *L)
{ecs_lua_dbg("QUERY_iter");
    ecs_world_t *w = ecs_lua_world(L);
    ecs_query_t *query = checkquery(L, 1);

    ecs_lua_check_world(L, w, 1);

    ecs_iter_t it = ecs_query_iter(w, query);

    /* will push with no columns because it->count = 0 */
    ecs_iter_to_lua(&it, L, true);

    return 1;
}

int query_next(lua_State *L)
{
    ecs_iter_t *it = ecs_lua_to_iter(L, 1);
    int b = ecs_query_next(it);

    if(b) ecs_lua_iter_update(L, 1, it);

    lua_pushboolean(L, b);

    return 1;
}

int query_changed(lua_State *L)
{
    ecs_query_t *query = checkquery(L, 1);

    int b = ecs_query_changed(query, NULL);

    lua_pushboolean(L, b);

    return 1;
}
