#include "private.h"

static void print_time(ecs_time_t *time, const char *str)
{
#ifndef NDEBUG
    double sec = ecs_time_measure(time);
    ecs_lua_dbg("Lua %s took %f milliseconds", str, sec * 1000.0);
#endif
}

/* Also used for triggers */
static void system_entry_point(ecs_iter_t *it)
{
    ecs_lua_system *sys = it->param;
    lua_State *L = sys->L;
    ecs_world_t *w = it->world;
    const ecs_world_t *real_world = ecs_get_world(w);

    /* Since >2.3.2 it->world != the actual world, we have to
       swap the world pointer for all API calls with it->world (stage pointer)
    */
    ecs_lua__prolog(L);

    int ret = lua_rawgetp(L, LUA_REGISTRYINDEX, real_world);
    ecs_assert(ret == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    ret = lua_rawgeti(L, -1, ECS_LUA_APIWORLD);
    ecs_assert(ret == LUA_TUSERDATA, ECS_INTERNAL_ERROR, NULL);

    ecs_world_t **wbuf = lua_touserdata(L, -1);
    ecs_world_t *prev_world = *wbuf;

    lua_pop(L, 2);

    *wbuf = it->world;

    ecs_time_t time;
    int idx = ecs_get_thread_index(w);
    //int stage_id = ecs_get_stage_id(w);

    ecs_assert(idx == 0, ECS_INTERNAL_ERROR, "Lua systems must run on the main thread");

    ecs_lua_dbg("Lua %s: \"%s\", %d columns, count %d, func ref %d",
        sys->trigger ? "trigger" : "system", ecs_get_name(w, it->system),
        it->column_count, it->count, sys->func_ref);

    int type = lua_rawgeti(L, LUA_REGISTRYINDEX, sys->func_ref);

    ecs_assert(type == LUA_TFUNCTION, ECS_INTERNAL_ERROR, NULL);

    ecs_os_get_time(&time);

    ecs_iter_to_lua(it, L, false);

    print_time(&time, "iter serialization");

    lua_pushvalue(L, -1);
    int it_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    ecs_os_get_time(&time);

    ret = lua_pcall(L, 1, 0, 0);

    *wbuf = prev_world;

    print_time(&time, "system");

    if(ret)
    {
        const char *name = ecs_get_name(w, it->system);
        const char *err = lua_tostring(L, lua_gettop(L));
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

static int new_whatever(lua_State *L, ecs_world_t *w, bool trigger)
{
    ecs_lua_ctx *ctx = ecs_lua_get_context(L, w);

    ecs_entity_t e = 0;
    luaL_checktype(L, 1, LUA_TFUNCTION);
    const char *name = luaL_checkstring(L, 2);
    ecs_entity_t phase = luaL_optinteger(L, 3, 0);
    const char *signature = luaL_optstring(L, 4, NULL);

    if(trigger)
    {
        if(phase != EcsOnAdd && phase != EcsOnRemove) return luaL_argerror(L, 3, "invalid kind");
        if(signature == NULL) return luaL_argerror(L, 4, "missing signature");

        e = ecs_new_trigger(w, 0, name, phase, signature, system_entry_point);
    }
    else e = ecs_new_system(w, 0, name, phase, signature, system_entry_point);

    ecs_lua_system *sys = lua_newuserdata(L, sizeof(ecs_lua_system));
    luaL_ref(L, LUA_REGISTRYINDEX);

    sys->L = L;

    lua_pushvalue(L, 1);
    sys->func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    sys->param_ref = LUA_NOREF;
    sys->trigger = trigger;

    ecs_set(w, e, EcsName, {.alloc_value = (char*)name});
    ecs_set(w, e, EcsContext, { sys });

    lua_pushinteger(L, e);

    return 1;
}

int new_system(lua_State *L)
{
    return new_whatever(L, ecs_lua_world(L), false);
}

int new_trigger(lua_State *L)
{
    return new_whatever(L, ecs_lua_world(L), true);
}

int run_system(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t system = luaL_checkinteger(L, 1);
    lua_Number delta_time = luaL_checknumber(L, 2);

    const EcsContext *ctx = ecs_get(w, system, EcsContext);
    ecs_lua_system sys = *(ecs_lua_system*)ctx->ctx;

    int tmp = sys.param_ref;

    if(!lua_isnoneornil(L, 3)) sys.param_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    ecs_entity_t ret = ecs_run(w, system, delta_time, &sys);

    if(tmp != sys.param_ref) luaL_unref(L, LUA_REGISTRYINDEX, sys.param_ref);

    lua_pushinteger(L, ret);

    return 1;
}

int set_system_context(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t system = luaL_checkinteger(L, 1);
    if(lua_gettop(L) < 2) lua_pushnil(L);

    EcsContext *ctx = ecs_get_mut(w, system, EcsContext, NULL);
    ecs_lua_system *sys = (ecs_lua_system*)ctx->ctx;

    luaL_unref(L, LUA_REGISTRYINDEX, sys->param_ref);

    sys->param_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    ecs_modified(w, system, EcsContext);

    return 0;
}
