#include "private.h"


ecs_type_t checktype(lua_State *L, int arg)
{
    ecs_type_t *type = luaL_checkudata(L, arg, "ecs_type_t");

    return *type;
}

ecs_filter_t checkfilter(lua_State *L, int arg)
{
    luaL_checktype(L, arg, LUA_TTABLE);

    ecs_filter_t filter = {0};

    int include_type = lua_getfield(L, arg, "include");
    int exclude_type = lua_getfield(L, arg, "exclude");
    int include_kind_type = lua_getfield(L, arg, "include_kind");
    int exclude_kind_type = lua_getfield(L, arg, "exclude_kind");

    if(include_type != LUA_TNIL)
    {
        if(include_type != LUA_TUSERDATA) luaL_argerror(L, arg, "expected ecs_type_t for 'include'");

        filter.include = checktype(L, -4);
    }

    if(exclude_type != LUA_TNIL)
    {
        if(include_type != LUA_TUSERDATA) luaL_argerror(L, arg, "expected ecs_type_t for 'exclude'");

        filter.exclude = checktype(L, -3);
    }

    if(include_kind_type != LUA_TNIL)
    {
        if(!lua_isinteger(L, -2)) luaL_argerror(L, arg, "expected integer for 'include_kind'");

        filter.include_kind = luaL_checkinteger(L, -2);

        if(filter.include_kind < 0 || filter.include_kind > 3)
            luaL_argerror(L, 1, "invalid enum for include_kind");
    }

    if(exclude_kind_type != LUA_TNIL)
    {
        if(!lua_isinteger(L, -1)) luaL_argerror(L, arg, "expected integer for 'exclude_kind'");

        filter.exclude_kind = luaL_checkinteger(L, -1);

        if(filter.exclude_kind < 0 || filter.exclude_kind > 3)
            luaL_argerror(L, 1, "invalid enum for exclude_kind");
    }

    lua_pop(L, 4);

    return filter;
}

int assert_func(lua_State *L)
{
    if(lua_toboolean(L, 1)) return lua_gettop(L);

#ifdef NDEBUG
    return lua_gettop(L);
#endif

    luaL_checkany(L, 1);
    lua_remove(L, 1);
    lua_pushliteral(L, "assertion failed!");
    lua_settop(L, 1);

    int level = (int)luaL_optinteger(L, 2, 1);
    lua_settop(L, 1);

    if(lua_type(L, 1) == LUA_TSTRING && level > 0)
    {
        luaL_where(L, level);
        lua_pushvalue(L, 1);
        lua_concat(L, 2);
    }

    return lua_error(L);
}

int sizeof_component(lua_State *L)
{
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);
    ecs_world_t *w = ctx->world;

    ecs_entity_t e = luaL_checkinteger(L, 1);

    const EcsComponent *ptr = ecs_get(w, e, EcsComponent);

    lua_pushinteger(L, ptr->size);

    return 1;
}

int createtable(lua_State *L)
{
    int narr = luaL_optinteger(L, 1, 0);
    int nrec = luaL_optinteger(L, 2, 0);

    lua_createtable(L, narr, nrec);

    return 1;
}

int ecs_lua__readonly(lua_State *L)
{
    return luaL_error(L, "Attempt to modify read-only table");
}