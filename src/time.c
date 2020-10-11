#include "private.h"

static ecs_time_t checktime(lua_State *L, int arg)
{
    ecs_time_t time;

    lua_getfield(L, arg, "sec");
    time.sec = luaL_checkinteger(L, -1);

    lua_pop(L, 1);

    lua_getfield(L, arg, "nanosec");
    time.nanosec = luaL_checkinteger(L, -1);

    return time;
}
static void pushtime(lua_State *L, ecs_time_t *time)
{
    lua_createtable(L, 0, 2);

    lua_pushinteger(L, time->sec);
    lua_setfield(L, -2, "sec");

    lua_pushinteger(L, time->nanosec);
    lua_setfield(L, -2, "nanosec");
}

int get_time(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_time_t time = {0};

    ecs_os_get_time(&time);

    pushtime(L, &time);

    return 1;
}

int time_measure(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_time_t start = checktime(L, 1);

    double time = ecs_time_measure(&start);

    lua_pushnumber(L, time);

    return 1;
}