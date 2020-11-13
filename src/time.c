#include "private.h"

static ecs_time_t checktime(lua_State *L, int arg)
{
    lua_getfield(L, arg, "sec");
    lua_Integer sec = luaL_checkinteger(L, -1);
    if(sec > INT32_MAX) luaL_argerror(L, arg, "sec field out of range");

    lua_pop(L, 1);

    lua_getfield(L, arg, "nanosec");
    lua_Integer nanosec = luaL_checkinteger(L, -1);
    if(nanosec > INT32_MAX) luaL_argerror(L, arg, "nanosec out of range");

    ecs_time_t time =
    {
        .sec = (uint32_t)sec,
        .nanosec = (uint32_t)nanosec
    };

    return time;
}

int time__tostring(lua_State *L)
{
    ecs_time_t time = checktime(L, 1);

    double sec = ecs_time_to_double(time);

    char str[32];
    snprintf(str, sizeof(str), "%f", sec);

    lua_pushstring(L, str);

    return 1;
}

static void pushtime(lua_State *L, ecs_time_t *time)
{
    lua_createtable(L, 0, 3);

    lua_pushinteger(L, time->sec);
    lua_setfield(L, -2, "sec");

    lua_pushinteger(L, time->nanosec);
    lua_setfield(L, -2, "nanosec");

    luaL_setmetatable(L, "ecs_time_t");
}

int get_time(lua_State *L)
{
    ecs_time_t time = {0};

    ecs_os_get_time(&time);

    pushtime(L, &time);

    return 1;
}

int time_measure(lua_State *L)
{
    ecs_time_t start = checktime(L, 1);

    double time = ecs_time_measure(&start);

    lua_pushnumber(L, time);

    return 1;
}