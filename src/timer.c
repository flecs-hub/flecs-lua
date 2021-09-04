#include "private.h"


int set_timeout(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t timer = luaL_checkinteger(L, 1);
    lua_Number timeout = luaL_checknumber(L, 2);

    ecs_entity_t e = ecs_set_timeout(w, timer, timeout);

    lua_pushinteger(L, e);

    return 1;
}

int get_timeout(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t timer = luaL_checkinteger(L, 1);

    lua_Number timeout = ecs_get_timeout(w, timer);

    lua_pushnumber(L, timeout);

    return 1;
}

int set_interval(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t timer = luaL_checkinteger(L, 1);
    lua_Number interval = luaL_checknumber(L, 2);

    ecs_entity_t e = ecs_set_interval(w, timer, interval);

    lua_pushinteger(L, e);

    return 1;
}

int get_interval(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t timer = luaL_checkinteger(L, 1);

    lua_Number interval = ecs_get_interval(w, timer);

    lua_pushnumber(L, interval);

    return 1;
}

int start_timer(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t timer = luaL_checkinteger(L, 1);

    ecs_start_timer(w, timer);

    return 0;
}

int stop_timer(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t timer = luaL_checkinteger(L, 1);

    ecs_stop_timer(w, timer);

    return 0;
}

int set_rate_filter(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t filter = luaL_checkinteger(L, 1);
    lua_Integer rate = luaL_checkinteger(L, 2);
    ecs_entity_t src = luaL_checkinteger(L, 3);

    ecs_entity_t e = ecs_set_rate(w, filter, rate, src);

    lua_pushinteger(L, e);

    return 1;
}

int set_tick_source(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t system = luaL_checkinteger(L, 1);
    ecs_entity_t source = luaL_checkinteger(L, 2);

    ecs_set_tick_source(w, system, source);

    return 0;
}