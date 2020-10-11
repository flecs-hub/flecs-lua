#include "private.h"


ecs_iter_t *ecs_lua__checkiter(lua_State *L, int idx)
{
    if(luaL_getmetafield(L, idx, "__ecs_iter") != LUA_TTABLE) luaL_error(L, "table is not an iterator");

    lua_rawgeti(L, -1, 1);
    ecs_iter_t *it = lua_touserdata(L, -1);
    lua_pop(L, 2);

    return it;
}

ecs_iter_t *get_iter_columns(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_iter_t *it = ecs_lua__checkiter(L, 1);

    luaL_getsubtable(L, 1, "columns");

    return it;
}

int column(lua_State *L)
{
    ecs_iter_t *it = get_iter_columns(L);

    lua_Integer i = luaL_checkinteger(L, 2);

    if(i < 1 || i > it->column_count) luaL_argerror(L, 2, "invalid column index");

    lua_rawgeti(L, -1, i);

    return 1;
}

int columns(lua_State *L)
{
    ecs_iter_t *it = get_iter_columns(L);

    int i;
    for(i=1; i <= it->column_count; i++)
    {
        lua_rawgeti(L, 2, i);
    }

    return it->column_count;
}