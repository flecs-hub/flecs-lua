#include "private.h"

static void print_time(ecs_time_t *time, const char *str)
{
#ifndef NDEBUG
    double sec = ecs_time_measure(time);
    ecs_lua_dbg("Lua %s took %f milliseconds", str, sec * 1000.0);
#endif
}

static ecs_world_t **world_buf(lua_State *L, const ecs_world_t *world)
{
    int ret = lua_rawgetp(L, LUA_REGISTRYINDEX, world);
    ecs_assert(ret == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    ret = lua_rawgeti(L, -1, ECS_LUA_APIWORLD);
    ecs_assert(ret == LUA_TUSERDATA, ECS_INTERNAL_ERROR, NULL);

    ecs_world_t **wbuf = lua_touserdata(L, -1);
    ecs_world_t *prev_world = *wbuf;

    lua_pop(L, 2);

    return wbuf;
}

/* Used for systems, triggers and observers */
static void ecs_lua__callback(ecs_iter_t *it)
{
    ecs_assert(it->binding_ctx != NULL, ECS_INTERNAL_ERROR, NULL);

    ecs_world_t *w = it->world;
    ecs_lua_callback *cb = it->binding_ctx;
    const ecs_world_t *real_world = ecs_get_world(it->world);

    int stage_id = ecs_get_stage_id(w);
    int stage_count = ecs_get_stage_count(w);
    const char *name = ecs_get_name(it->world, it->system);

    ecs_assert(stage_id == 0, ECS_INTERNAL_ERROR, "Lua callbacks must run on the main thread");

    const EcsLuaHost *host = ecs_singleton_get(w, EcsLuaHost);
    ecs_assert(host != NULL, ECS_INVALID_PARAMETER, NULL);

    ecs_lua_dbg("Lua %s: \"%s\", %d terms, count %d, func ref %d",
            cb->type_name, name, it->field_count, it->count, cb->func_ref);

    lua_State *L = host->L; // host->states[stage_id];

    ecs_lua_ctx *ctx = ecs_lua_get_context(L, real_world);

    ecs_lua__prolog(L);

    /* Since >2.3.2 it->world != the actual world, we have to
       swap the world pointer for all API calls with it->world (stage pointer)
    */
    ecs_world_t **wbuf = world_buf(L, real_world);

    ecs_world_t *prev_world = *wbuf;
    *wbuf = it->world;

    ecs_time_t time;

    int type = ecs_lua_rawgeti(L, w, cb->func_ref);

    ecs_assert(type == LUA_TFUNCTION, ECS_INTERNAL_ERROR, NULL);

    ecs_os_get_time(&time);

    ecs_iter_to_lua(it, L, false);

    print_time(&time, "iter serialization");

    lua_pushvalue(L, -1);
    int it_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    ecs_os_get_time(&time);

    int ret = lua_pcall(L, 1, 0, 0);

    *wbuf = prev_world;

    print_time(&time, "system");

    if(ret)
    {
        const char *err = lua_tostring(L, lua_gettop(L));
        /* TODO: switch to ecs_os_err() with message handler */
        ecs_lua_dbg("error in %s callback \"%s\" (%d): %s", cb->type_name, name, ret, err);
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

static int check_events(lua_State *L, ecs_world_t *w, ecs_entity_t *events, int arg)
{
    ecs_entity_t event = 0;

    int type = lua_type(L, arg);

    if(type == LUA_TTABLE)
    {
        int len = luaL_len(L, arg);

        if(len > FLECS_EVENT_DESC_MAX) return luaL_argerror(L, arg, "too many events");

        int i;
        for(i=1; i <= len; i++)
        {
            type = lua_rawgeti(L, arg, i);

            if(type != LUA_TNUMBER) return luaL_argerror(L, arg, "invalid event");

            event = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            if(!event || !ecs_is_valid(w, event)) return luaL_argerror(L, arg, "invalid event");

            events[i - 1] = event;
        }
    }
    else if(type == LUA_TNUMBER)
    {
        event = luaL_checkinteger(L, arg);

        if(!event || !ecs_is_valid(w, event)) return luaL_argerror(L, arg, "invalid event");

        events[0] = event;
    }
    else return luaL_argerror(L, arg, "invalid event type");

    return 1;
}

static int new_callback(lua_State *L, ecs_world_t *w, enum EcsLuaCallbackType type)
{
    ecs_lua_ctx *ctx = ecs_lua_get_context(L, w);

    ecs_entity_t e = 0;
    luaL_checktype(L, 1, LUA_TFUNCTION);
    const char *name = luaL_optstring(L, 2, NULL);
    /* phase, event or event[] expected for arg 3 */
    const char *signature = lua_type(L, 4) == LUA_TSTRING ? luaL_checkstring(L, 4) : NULL;

    ecs_lua_callback *cb = lua_newuserdata(L, sizeof(ecs_lua_callback));

    ecs_lua_ref(L, w);

    if(type == EcsLuaObserver)
    {
        ecs_entity_desc_t edesc = { .name = name };
        e = ecs_entity_init(w, &edesc);

        ecs_observer_desc_t desc =
        {
            .entity = e,
            .callback = ecs_lua__callback,
            .filter.expr = signature,
            .binding_ctx = cb
        };

        if(signature == NULL) check_filter_desc(L, w, &desc.filter, 4);

        check_events(L, w, desc.events, 3);

        e = ecs_observer_init(w, &desc);

        cb->type_name = "observer";
    }
    else /* EcsLuaSystem */
    {
        ecs_entity_t phase = luaL_checkinteger(L, 3);

        ecs_entity_desc_t edesc = {0};
        edesc.name = name;
        edesc.add[0] = phase ? ecs_dependson(phase) : 0;
        edesc.add[1] = phase;
        e = ecs_entity_init(w, &edesc);

        ecs_system_desc_t desc = {0};
        desc.entity = e;
        desc.query.filter.expr = signature;
        desc.callback = ecs_lua__callback;
        desc.binding_ctx = cb;

        if(signature == NULL && !lua_isnoneornil(L, 4)) check_filter_desc(L, w, &desc.query.filter, 4);

        e = ecs_system_init(w, &desc);

        cb->type_name = "system";
    }

    if(!e) return luaL_error(L, "failed to create %s", cb->type_name);

    lua_pushvalue(L, 1);
    cb->func_ref = ecs_lua_ref(L, w);
    cb->param_ref = LUA_NOREF;
    cb->type = type;

    lua_pushinteger(L, e);

    return 1;
}

int new_system(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
    return new_callback(L, w, EcsLuaSystem);
}

int new_trigger(lua_State *L)
{
    return luaL_error(L, "use ecs.observer()");
}

int new_observer(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
    return new_callback(L, w, EcsLuaObserver);
}

int run_system(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t system = luaL_checkinteger(L, 1);
    lua_Number delta_time = luaL_checknumber(L, 2);

    ecs_lua_callback *sys = ecs_get_system_binding_ctx(w, system);

    if(sys == NULL) return luaL_argerror(L, 1, "not a Lua system");

    int tmp = sys->param_ref;

    if(!lua_isnoneornil(L, 3)) sys->param_ref = ecs_lua_ref(L, w);

    ecs_entity_t ret = ecs_run(w, system, delta_time, NULL);

    if(tmp != sys->param_ref)
    {/* Restore previous value */
        ecs_lua_unref(L, w, sys->param_ref);
        sys->param_ref = tmp;
    }

    lua_pushinteger(L, ret);

    return 1;
}

int set_system_context(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t system = luaL_checkinteger(L, 1);
    if(lua_gettop(L) < 2) lua_pushnil(L);

    ecs_lua_callback *sys = ecs_get_system_binding_ctx(w, system);

    ecs_lua_unref(L, w, sys->param_ref);

    sys->param_ref = ecs_lua_ref(L, w);

    return 0;
}
