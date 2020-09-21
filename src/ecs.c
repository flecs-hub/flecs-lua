#include "private.h"

#define ecs_lua__prolog(L) int ecs_lua__stackguard = lua_gettop(L)
#define ecs_lua__epilog(L) ecs_assert(ecs_lua__stackguard == lua_gettop(L), ECS_INTERNAL_ERROR, NULL)

#define ECS_LUA__KEEPOPEN 1

typedef struct ecs_lua_system
{
    lua_State *L;
    int func_ref;
    const char *signature;
}ecs_lua_system;

typedef struct ecs_lua_module
{
    ecs_lua_ctx *ctx;
    const char *name;
    ecs_entity_t e;
}ecs_lua_module;

ecs_lua_ctx *ecs_lua_get_context(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "ecs_lua");
    ecs_lua_ctx *p = lua_touserdata(L, -1);
    lua_pop(L, 1);
    return p;
}

ecs_world_t *ecs_lua_get_world(lua_State *L)
{
    ecs_lua_ctx *p = ecs_lua_get_context(L);
    return p->world;
}

static void print_time(ecs_time_t *time, const char *str)
{
#ifndef NDEBUG
    double sec = ecs_time_measure(time);
    ecs_os_dbg("Lua %s took %f milliseconds", str, sec * 1000.0);
#endif
}

static void entry_point(ecs_iter_t *it)
{
    ecs_world_t *w = it->world;
    ecs_lua_system *sys = it->param;
    lua_State *L = sys->L;
    ecs_lua__prolog(L);

    ecs_entity_t e = 0;
    ecs_entity_t type_entity = 0;
    int nargs = it->column_count;
    int idx = ecs_get_thread_index(w);

    ecs_time_t time;

    ecs_assert(!strcmp(sys->signature, ecs_get(w, it->system, EcsSignatureExpr)->expr), ECS_INTERNAL_ERROR, NULL);

    ecs_os_dbg("Lua system: \"%s\", %d columns, func ref %d", ecs_get_name(w, it->system), nargs, sys->func_ref);

    ecs_os_get_time(&time);

    int i, k, col;
    for(col=0, i=0; i < nargs; col++, i++)
    {
        lua_createtable(L, it->count, 0);

        //type_entity = ecs_type_from_entity(w, it->entities[i]);
        //ecs_iter_to_lua(w, it, type_entity, L);

        if(ecs_is_readonly(it, col))
        {

        }
    }

    print_time(&time, "iter serialization");

    ecs_os_get_time(&time);

    int type = lua_rawgeti(L, LUA_REGISTRYINDEX, sys->func_ref);

    luaL_checktype(L, -1, LUA_TFUNCTION);

    int ret = lua_pcall(L, nargs, nargs, 0);

    print_time(&time, "system");

    for(col=0, i=-nargs; i < 0; col++, i++)
    {
        luaL_checktype(L, i, LUA_TTABLE);
        if(it->count == lua_rawlen(L, -1))
            luaL_error(L, "expected %d elements in column %d, got %d", it->count, -i, lua_rawlen(L, -1));

        if(ecs_is_readonly(it, col))
        {
            continue;
        }

        //type_entity = ecs_type_from_entity(w, it->entities[i]);
        //ecs_lua_to_iter(w, it, type_entity, L);
    }
    ecs_lua__epilog(L);
}

static void set_default_name(ecs_world_t *w, ecs_entity_t e)
{
#ifndef NDEBUG
    char str[32];
    snprintf(str, sizeof(str), "Lua.%llu", e);
    ecs_set(w, e, EcsName, {.alloc_value = str});
#else
    ecs_set(w, e, EcsName, {.value = "Lua.Entity"});
#endif
}

static int new_entity(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t e = 0;

    const char *name = NULL;
    const char *components = NULL;
    int args = lua_gettop(L);

    if(!args)
    {
        e = ecs_new(w, 0);
    }
    else if(args == 1)
    {
        if(lua_isinteger(L, 1)) e = luaL_checkinteger(L, 1);
        else name = luaL_checkstring(L, 1);
    }
    else if(args == 2)
    {
        if(lua_isinteger(L, 1))
        {
            e = luaL_checkinteger(L, 1);
            name = luaL_checkstring(L, 2);
        }
        else
        {
            name = luaL_checkstring(L, 1);
            components = luaL_checkstring(L, 2);
        }
    }
    else if(args == 3)
    {
        e = luaL_checkinteger(L, 1);
        name = luaL_checkstring(L, 2);
        components = luaL_checkstring(L, 3);
    }
    else return luaL_error(L, "too many arguments");

    if(name)
    {
        e = ecs_new_entity(w, e, name, components);
        ecs_set(w, e, EcsName, {.alloc_value = (char*)name});
    }
    else set_default_name(w, e);

    lua_pushinteger(L, e);

    return 1;
}

static int bulk_new(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    lua_Integer count = 0;
    const char *name = NULL;
    const ecs_entity_t* entities = NULL;

    if(lua_gettop(L) == 2)
    {
        name = luaL_checkstring(L, 1);
        count = luaL_checkinteger(L, 2);

        ecs_entity_t type_entity = ecs_lookup(w, name);

        if(!type_entity) return luaL_argerror(L, 2, "could not find type");

        ecs_type_t type = ecs_type_from_entity(w, type_entity);

        entities = ecs_bulk_new_w_type(w, type, count);
    }
    else
    {
        count = luaL_checkinteger(L, 1);
        entities = ecs_bulk_new(w, 0, count);
    }

    lua_newtable(L);

    lua_Integer i;
    for(i=0; i < count; i++)
    {
        lua_pushinteger(L, entities[i]);
        lua_rawseti(L, -2, i+1);

        set_default_name(w, entities[i]);
    }

    return 1;
}

static int delete_entity(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t entity;

    if(lua_isinteger(L, 1)) entity = luaL_checkinteger(L, 1);
    else
    {
        const char *name = luaL_checkstring(L, 1);
        entity = ecs_lookup_fullpath(w, name);

        if(!entity) return luaL_argerror(L, 1, "could not find entity");
    }

    ecs_delete(w, entity);

    return 0;
}

static int new_tag(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    const char *name = luaL_checkstring(L, 1);

    ecs_entity_t e = 0;

    e = ecs_new_entity(w, e, name, NULL);
    ecs_set(w, e, EcsName, {.alloc_value = (char*)name});

    lua_pushinteger(L, e);

    return 1;
}

static int entity_name(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    const char *name = ecs_get_name(w, e);

    lua_pushstring(L, name);

    return 1;
}

static int lookup_entity(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    const char *name = luaL_checkstring(L, 1);

    ecs_entity_t e = ecs_lookup(w, name);

    lua_pushinteger(L, e);

    return 1;
}

static int lookup_fullpath(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    const char *name = luaL_checkstring(L, 1);

    ecs_entity_t e = ecs_lookup_fullpath(w, name);

    lua_pushinteger(L, e);

    return 1;
}

static int entity_has(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t type_entity = 0;
    ecs_type_t type = NULL;

    if(lua_isinteger(L, 2)) type_entity = luaL_checkinteger(L, 2);
    else
    {
        const char *name = luaL_checkstring(L, 2);
        type_entity = ecs_lookup_fullpath(w, name);

        if(!type_entity) return luaL_argerror(L, 2, "could not find type");
    }

    type = ecs_type_from_entity(w, type_entity);

    if(ecs_has_type(w, e, type)) lua_pushboolean(L, 1);
    else lua_pushboolean(L, 0);

    return 1;
}

static int has_role(lua_State *L)
{
    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t role = luaL_checkinteger(L, 2);

    if((e & ECS_ROLE_MASK) == role) lua_pushboolean(L, 1);
    else lua_pushboolean(L, 0);

    return 1;
}

static int add_type(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t type_entity = 0;

    if(lua_isinteger(L, 2)) type_entity = luaL_checkinteger(L, 2);
    else
    {
        const char *name = luaL_checkstring(L, 2);
        type_entity = ecs_lookup_fullpath(w, name);

        if(!type_entity) return luaL_argerror(L, 2, "could not find type");
    }

    ecs_type_t type = ecs_type_from_entity(w, type_entity);

    ecs_add_type(w, e, type);

    return 0;
}

static int remove_type(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t type_entity = 0;

    if(lua_isinteger(L, 2)) type_entity = luaL_checkinteger(L, 2);
    else
    {
        const char *name = luaL_checkstring(L, 2);
        type_entity = ecs_lookup_fullpath(w, name);

        if(!type_entity) return luaL_argerror(L, 2, "could not find type");
    }

    ecs_type_t type = ecs_type_from_entity(w, type_entity);

    ecs_remove_type(w, e, type);

    return 0;
}

static int clear_entity(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t e = lua_tointeger(L, 1);

    ecs_clear(w, e);

    return 0;
}

static int new_array(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *desc = luaL_checkstring(L, 2);

    ecs_entity_t ecs_entity(EcsMetaType) = ecs_lookup_fullpath(w, "flecs.meta.MetaType");

    ecs_entity_t e = 0;

    e = ecs_set(w, 0, EcsMetaType,
    {
        .kind = EcsArrayType,
        .size = 0,
        .alignment = 0,
        .descriptor = desc
    });

    ecs_set(w, e, EcsName, {.alloc_value = (char*)name});

    lua_pushinteger(L, e);

    return 1;
}

static int new_struct(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *desc = luaL_checkstring(L, 2);

    ecs_entity_t ecs_entity(EcsMetaType) = ecs_lookup_fullpath(w, "flecs.meta.MetaType");

    ecs_entity_t e = 0;

    e = ecs_set(w, 0, EcsMetaType,
    {
        .kind = EcsStructType,
        .size = 0,
        .alignment = 0,
        .descriptor = desc
    });

    ecs_set(w, e, EcsName, {.alloc_value = (char*)name});

    lua_pushinteger(L, e);

    return 1;
}

static int new_alias(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *alias = luaL_checkstring(L, 2);

    ecs_entity_t ecs_entity(EcsMetaType) = ecs_lookup_fullpath(w, "flecs.meta.MetaType");

    ecs_entity_t type_entity = ecs_lookup_fullpath(w, name);

    if(!type_entity) return luaL_argerror(L, 1, "unknown name");

    const EcsMetaType *p = ecs_get(w, type_entity, EcsMetaType);

    if(!p) return luaL_argerror(L, 1, "missing descriptor");

    EcsMetaType meta = *p;

    ecs_entity_t e = ecs_new_component(w, 0, NULL, meta.size, meta.alignment);

    ecs_set(w, e, EcsName, {.alloc_value = (char*)alias});

    ecs_new_meta(w, e, &meta);

    lua_pushinteger(L, e);

    return 1;
}

static int get_func(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    const void *ptr = ecs_get_w_entity(w, e, component);

    if(ptr) ecs_lua_push_ptr(w, L, component, ptr);
    else lua_pushnil(L);

    return 1;
}

static int get_mut(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    bool is_added = 0;
    void *ptr = ecs_get_mut_w_entity(w, e, component, &is_added);

    lua_pushboolean(L, (int)is_added);

    if(ptr) ecs_lua_push_ptr(w, L, component, ptr);
    else lua_pushnil(L);

    return 1;
}

static int new_system(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);

    lua_pushvalue(L, 1); /* luaL_ref() pops from the stack */
    luaL_checktype(L, 1, LUA_TFUNCTION);

    int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    const char *name = luaL_checkstring(L, 2);
    ecs_entity_t phase = luaL_optinteger(L, 3, 0);
    const char *signature = luaL_optstring(L, 4, "0");

    ecs_entity_t e = ecs_new_system(w, 0, name, phase, signature, entry_point);

    ecs_lua_system *sys = lua_newuserdata(L, sizeof(ecs_lua_system));
    luaL_ref(L, LUA_REGISTRYINDEX);

    lua_pushstring(L, signature);
    int sig = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_rawgeti(L, LUA_REGISTRYINDEX, sig);

    sys->L = L;
    sys->func_ref = func_ref;
    sys->signature = lua_tostring(L, -1);

    ecs_set(w, e, EcsName, {.alloc_value = (char*)name});
    ecs_set(w, e, EcsContext, { sys });

    lua_pushinteger(L, e);

    return 1;
}

void import_func(ecs_world_t *w)
{
    ecs_lua_module *m = ecs_get_context(w);
    ecs_lua_ctx *ctx = m->ctx;
    lua_State *L = ctx->L;

    ecs_os_dbg("ecs_lua: import callback");

    m->e = ecs_new_module(w, 0, m->name, 4, 4);

    ecs_set(w, m->e, EcsName, {.alloc_value = (char*)m->name});

    ctx->error = lua_pcall(L, 0, 0, 0);
}

static int new_module(lua_State *L)
{
    ecs_lua__prolog(L);
    ecs_world_t *w = ecs_lua_get_world(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);

    const char *name = luaL_checkstring(L, 1);

    luaL_checktype(L, 2, LUA_TFUNCTION);

    ecs_lua_module m = { .ctx = ctx, .name = name };

    void *orig = ecs_get_context(w);
    ecs_set_context(w, &m);

    ecs_import(w, import_func, name, NULL, 0);

    ecs_set_context(w, orig);

    ecs_assert(!ctx->error, ECS_INTERNAL_ERROR, lua_tostring(L, -1));

    if(ctx->error) return lua_error(L);

    lua_pushinteger(L, m.e);

    ecs_lua__epilog(L);
    return 1;
}

static int import_module(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);

    const char *name = luaL_checkstring(L, 1);
    ecs_entity_t e = 0;

    luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
    lua_getfield(L, -1, name);

    if(!lua_toboolean(L, -1))
    {
        lua_getglobal(L, "require");
        lua_pushstring(L, name);
        int ret = lua_pcall(L, 1, 1, 0);
        if(ret) lua_error(L);

        lua_getfield(L, -1, "import");
        luaL_checktype(L, -1, LUA_TFUNCTION);

        ecs_world_t *orig = ecs_get_context(w);
        ecs_set_context(w, ctx);

        e = ecs_import(w, import_func, name, NULL, 0);

        ecs_set_context(w, orig);

        ecs_assert(!ctx->error, ECS_INTERNAL_ERROR, lua_tostring(L, -1));

        if(ctx->error) return lua_error(L);

        ecs_set(w, e, EcsName, {.alloc_value = (char*)name});
    }


    luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
    lua_getfield(L, -1, name);

    return 1;
}

static int vararg2str(lua_State *L, int n, ecs_strbuf_t *buf)
{
    lua_getglobal(L, "tostring");

    int i;
    for(i=1; i <= n; i++)
    {
        lua_pushvalue(L, -1);
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);
        const char *arg = lua_tostring(L, -1);

        if(!arg) return luaL_error(L, "expected string from 'tostring'");

        if(i>1) ecs_strbuf_appendstr(buf, " ");

        ecs_strbuf_appendstr(buf, arg);

        lua_pop(L, 1);
    }

    return 0;
}

#define ECS_LUA__LOG 0
#define ECS_LUA__ERROR 1
#define ECS_LUA__DEBUG 2
#define ECS_LUA__WARN 3

static int print_type(lua_State *L, int type)
{
    int n = lua_gettop(L);

    ecs_strbuf_t buf = ECS_STRBUF_INIT;

    vararg2str(L, n, &buf);

    char *str = ecs_strbuf_get(&buf);

    switch(type)
    {
        case ECS_LUA__LOG:
            ecs_os_log(str);
            break;
        case ECS_LUA__ERROR:
            ecs_os_err(str);
            break;
        case ECS_LUA__DEBUG:
            ecs_os_dbg(str);
            break;
        case ECS_LUA__WARN:
            ecs_os_warn(str);
            break;
        default:
            break;
    }

    ecs_strbuf_reset(&buf);

    return 0;
}

static int print_log(lua_State *L)
{
    return print_type(L, ECS_LUA__LOG);
}

static int print_err(lua_State *L)
{
    return print_type(L, ECS_LUA__ERROR);
}

static int print_dbg(lua_State *L)
{
    return print_type(L, ECS_LUA__DEBUG);
}

static int print_warn(lua_State *L)
{
    return print_type(L, ECS_LUA__WARN);
}

static int assert_func(lua_State *L)
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

static int set_target_fps(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    lua_Number fps = luaL_checknumber(L, 1);

    ecs_set_target_fps(w, fps);

    return 0;
}

void ecs_lua_progress(lua_State *L)
{
    ecs_lua__prolog(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);

    ecs_assert(ctx->progress_ref != LUA_NOREF, ECS_INVALID_PARAMETER, "progress callback is not set");

    if(ctx->progress_ref == LUA_NOREF) return;

    lua_rawgeti(L, LUA_REGISTRYINDEX, ctx->progress_ref);

    int type = lua_type(L, -1);
    ecs_assert(type == LUA_TFUNCTION, ECS_INTERNAL_ERROR, NULL);

    if(type != LUA_TFUNCTION) return;

    lua_pcall(L, 0, 0, 0);

    ecs_lua__epilog(L);
}

static int progress(lua_State *L)
{
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);
    ecs_world_t *w = ctx->world;

    luaL_checktype(L, 1, LUA_TFUNCTION);
    ctx->progress_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}

static int world_info(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);
    const ecs_world_info_t *wi = ecs_get_world_info(w);

    ecs_entity_t e = ecs_lookup_fullpath(w, "flecs.lua.LuaWorldInfo");
    ecs_assert(e, ECS_INTERNAL_ERROR, NULL);

    struct EcsLuaWorldInfo world_info =
    {
        .last_component_id = wi->last_component_id,
        .last_id = wi->last_id,
        .min_id = wi->min_id,
        .max_id = wi->max_id,
        .delta_time_raw = wi->delta_time_raw,
        .delta_time = wi->delta_time,
        .time_scale = wi->time_scale,
        .target_fps = wi->target_fps,
        .frame_time_total = wi->frame_time_total,
        .system_time_total = wi->system_time_total,
        .merge_time_total = wi->merge_time_total,
        .world_time_total = wi->world_time_total,
        .world_time_total_raw = wi->world_time_total_raw,
        .sleep_err = wi->sleep_err,
        .frame_count_total = wi->frame_count_total,
        .merge_count_total = wi->merge_count_total,
        .pipeline_build_count_total = wi->pipeline_build_count_total,
        .systems_ran_frame = wi->systems_ran_frame,
    };

    ecs_lua_push_ptr(w, L, e, &world_info);

    return 1;
}

static int func(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);
    return 1;
}

static const luaL_Reg ecs_lib[] =
{
    { "new", new_entity },
    { "bulk_new", bulk_new },
    { "delete", delete_entity },
    { "tag", new_tag },
    { "name", entity_name },
    { "lookup", lookup_entity },
    { "lookup_fullpath", lookup_fullpath },
    { "has", entity_has },
    { "has_role", has_role },
    { "add", add_type },
    { "remove", remove_type },
    { "clear", clear_entity },
    { "array", new_array },
    { "struct", new_struct },
    { "alias", new_alias },

    { "get", get_func },
    { "get_mut", get_mut },

    { "system", new_system },
    { "module", new_module },
    { "import", import_module },

    { "log", print_log },
    { "err", print_err },
    { "dbg", print_dbg },
    { "warn", print_warn },
    { "assert", assert_func },

    { "set_target_fps", set_target_fps },
    { "progress", progress },
    { "world_info", world_info },

#define XX(const) {#const, NULL },
    ECS_LUA_ENUMS(XX)
    ECS_LUA_MACROS(XX)
#undef XX
    { NULL, NULL }
};

int luaopen_ecs(lua_State *L)
{
    luaL_newlib(L, ecs_lib);

#define XX(const) lua_pushinteger(L, Ecs##const); lua_setfield(L, -2, #const);
    ECS_LUA_ENUMS(XX)
#undef XX
#define XX(const) lua_pushinteger(L, ECS_##const); lua_setfield(L, -2, #const);
    ECS_LUA_MACROS(XX)
#undef XX
    return 1;
}

static ecs_lua_ctx * ctx_init(ecs_lua_ctx ctx)
{
    lua_State *L = ctx.L;

    ecs_lua_ctx *lctx = lua_newuserdata(L, sizeof(ecs_lua_ctx));
    lua_setfield(L, LUA_REGISTRYINDEX, "ecs_lua");

    memcpy(lctx, &ctx, sizeof(ecs_lua_ctx));

    lctx->error = 0;
    lctx->progress_ref = LUA_NOREF;

    luaL_requiref(L, "ecs", luaopen_ecs, 1);
    lua_pop(L, 1);

    return lctx;
}

int ecs_lua_init(ecs_lua_ctx ctx)
{
    if(ctx.world == NULL || ctx.L == NULL) return 1;

    ctx.internal = ECS_LUA__KEEPOPEN;

    ctx_init(ctx);

    return 0;
}

void ecs_lua_exit(lua_State *L)
{
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);

    if( !(ctx->internal & ECS_LUA__KEEPOPEN) ) lua_close(L);
}

int ecs_lua_set_state(ecs_world_t *w, lua_State *L)
{
    ecs_entity_t e = ecs_lookup_fullpath(w, "flecs.lua.LuaHost");

    EcsLuaHost *ctx = ecs_get_mut_w_entity(w, EcsSingleton, e, NULL);

    ecs_lua_exit(ctx->L);

    ecs_lua_ctx param = { .L = L, .world = w, .internal = ECS_LUA__KEEPOPEN };

    ctx = ctx_init(param);

    ecs_set_ptr_w_entity(w, EcsSingleton, e, sizeof(EcsLuaHost), ctx);

    return 0;
}

static void *Allocf(void *ud, void *ptr, size_t osize, size_t nsize)
{
    if(!nsize)
    {
        ecs_os_free(ptr);
        return NULL;
    }

    return ecs_os_realloc(ptr, nsize);
}

ECS_DTOR(EcsLuaHost, ctx,
{
    lua_State *L = ctx->L;
    ecs_lua_exit(L);
});

void FlecsLuaImport(ecs_world_t *w)
{
    ECS_MODULE(w, FlecsLua);

    ECS_IMPORT(w, FlecsMeta);

    ecs_set_name_prefix(w, "Ecs");

    ECS_COMPONENT(w, EcsLuaHost);

    ECS_META(w, EcsLuaWorldInfo);

    ECS_EXPORT_COMPONENT(EcsLuaHost);

    ecs_assert(sizeof(EcsLuaWorldInfo) == sizeof(ecs_world_info_t), ECS_INTERNAL_ERROR, NULL);

    lua_State *L = lua_newstate(Allocf, NULL);

    ecs_lua_ctx param = { .L = L, .world = w};
    ecs_lua_ctx *ctx = ctx_init(param);

    ecs_set_ptr(w, EcsSingleton, EcsLuaHost, ctx);

    ecs_set_component_actions(w, EcsLuaHost,
    {
        .dtor = ecs_dtor(EcsLuaHost)
    });
}
