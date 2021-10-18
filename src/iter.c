#include "private.h"


ecs_iter_t *ecs_lua__checkiter(lua_State *L, int arg)
{
    if(luaL_getmetafield(L, arg, "__ecs_iter") == LUA_TNIL)
        luaL_argerror(L, arg, "table is not an iterator");

    ecs_iter_t *it = lua_touserdata(L, -1);
    lua_pop(L, 1);

    return it;
}

ecs_term_t checkterm(lua_State *L, const ecs_world_t *world, int arg)
{
    ecs_term_t term = {0};

    luaL_checktype(L, arg, LUA_TNUMBER);

    term.id = luaL_checkinteger(L, arg);

    if(ecs_term_finalize(world, NULL, NULL, &term)) luaL_argerror(L, arg, "invalid term");

    return term;
}

ecs_iter_t *get_iter_columns(lua_State *L)
{
    ecs_iter_t *it = ecs_lua__checkiter(L, 1);

    luaL_getsubtable(L, 1, "columns");

    return it;
}

int column(lua_State *L)
{
    ecs_iter_t *it = get_iter_columns(L);

    lua_Integer i = luaL_checkinteger(L, 2);

    if(i < 1 || i > it->column_count) luaL_argerror(L, 2, "invalid column index");

    lua_geti(L, -1, i);

    return 1;
}

int columns(lua_State *L)
{
    ecs_iter_t *it = get_iter_columns(L);

    int i;
    for(i=1; i <= it->column_count; i++)
    {
        lua_geti(L, 2, i);
    }

    lua_getfield(L, 1, "entities");

    return it->column_count + 1;
}

int is_owned(lua_State *L)
{
    ecs_iter_t *it = ecs_lua__checkiter(L, 1);
    lua_Integer col = luaL_checkinteger(L, 2);

    if(col < 1 || col > it->column_count) luaL_argerror(L, 2, "invalid column index");

    int b = ecs_term_is_owned(it, col);

    lua_pushboolean(L, b);

    return 1;
}

int term_id(lua_State *L)
{
    ecs_iter_t *it = ecs_lua__checkiter(L, 1);
    lua_Integer col = luaL_checkinteger(L, 2);

    if(col < 1 || col > it->column_count) luaL_argerror(L, 2, "invalid column index");

    ecs_entity_t e = ecs_term_id(it, col);

    lua_pushinteger(L, e);

    return 1;
}

int filter_iter(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
    ecs_filter_t filter = checkfilter(L, 1);

    ecs_iter_t it = ecs_filter_iter(w, &filter);

    ecs_iter_to_lua(&it, L, true);

    return 1;
}

int filter_next(lua_State *L)
{
    ecs_iter_t *it = ecs_lua_to_iter(L, 1);

    int b = ecs_filter_next(it);

    if(b) ecs_lua_iter_update(L, 1, it);

    lua_pushboolean(L, b);

    return 1;
}

int term_iter(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_term_t term = checkterm(L, w, 1);

    ecs_iter_t it = ecs_term_iter(w, &term);

    ecs_iter_to_lua(&it, L, true);

    return 1;
}

int term_next(lua_State *L)
{
    ecs_iter_t *it = ecs_lua_to_iter(L, 1);

    int b = ecs_term_next(it);

    if(b) ecs_lua_iter_update(L, 1, it);

    lua_pushboolean(L, b);

    return 1;
}
