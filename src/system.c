#include "private.h"

typedef struct ecs_lua_system
{
    lua_State *L;
    int func_ref;
    bool trigger;
}ecs_lua_system;

static void print_time(ecs_time_t *time, const char *str)
{
#ifndef NDEBUG
    double sec = ecs_time_measure(time);
    ecs_os_dbg("Lua %s took %f milliseconds", str, sec * 1000.0);
#endif
}

/* Also used for triggers */
static void system_entry_point(ecs_iter_t *it)
{
    ecs_world_t *w = it->world;
    ecs_lua_system *sys = it->param;
    lua_State *L = sys->L;

    ecs_time_t time;
    int idx = ecs_get_thread_index(w);

    ecs_assert(idx == 0, ECS_INTERNAL_ERROR, "Lua systems must run on the main thread");

    ecs_os_dbg("Lua %s: \"%s\", %d columns, count %d, func ref %d",
        sys->trigger ? "trigger" : "system", ecs_get_name(w, it->system),
        it->column_count, it->count, sys->func_ref);

    ecs_lua__prolog(L);

    int type = lua_rawgeti(L, LUA_REGISTRYINDEX, sys->func_ref);

    ecs_assert(type == LUA_TFUNCTION, ECS_INTERNAL_ERROR, NULL);

    ecs_os_get_time(&time);

    ecs_iter_to_lua(it, L, false);

    print_time(&time, "iter serialization");

    lua_pushvalue(L, -1);
    int it_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    ecs_os_get_time(&time);

    int ret = lua_pcall(L, 1, 0, 0);

    print_time(&time, "system");

    if(ret)
    {
        const char *name = ecs_get_name(w, it->system);
        const char *err = lua_tostring(L, 2); /* 1 is the iterator */
        ecs_os_err("error running system \"%s\" (%d): %s", name, ret, err);
    }

    ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);

    lua_rawgeti(L, LUA_REGISTRYINDEX, it_ref);

    ecs_assert(lua_type(L, -1) == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    ecs_os_get_time(&time);

    ecs_lua_to_iter(L, -1);

    print_time(&time, "iter deserialization");

    luaL_unref(L, LUA_REGISTRYINDEX, it_ref);
    lua_pop(L, 1);

    ecs_lua__epilog(L);
}

int new_whatever(lua_State *L, bool trigger)
{
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);
    ecs_world_t *w = ctx->world;

    luaL_checktype(L, 1, LUA_TFUNCTION);
    const char *name = luaL_checkstring(L, 2);
    ecs_entity_t phase = luaL_optinteger(L, 3, 0);
    const char *signature = luaL_optstring(L, 4, NULL);

    ecs_entity_t e;

    if(trigger)
    {
        if(phase != EcsOnAdd && phase != EcsOnRemove) return luaL_argerror(L, 3, "invalid kind");
        if(signature == NULL) return luaL_argerror(L, 4, "");

        e = ecs_new_trigger(w, 0, name, phase, signature, system_entry_point);
    }
    else e = ecs_new_system(w, 0, name, phase, signature, system_entry_point);

    ecs_lua_system *sys = lua_newuserdata(L, sizeof(ecs_lua_system));
    luaL_ref(L, LUA_REGISTRYINDEX);

    sys->L = L;

    lua_pushvalue(L, 1);
    sys->func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    sys->trigger = trigger;

    ecs_set(w, e, EcsName, {.alloc_value = (char*)name});
    ecs_set(w, e, EcsContext, { sys });

    lua_pushinteger(L, e);

    return 1;
}

int new_system(lua_State *L)
{
    return new_whatever(L, false);
}

int new_trigger(lua_State *L)
{
    return new_whatever(L, true);
}