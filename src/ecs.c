#include <flecs_lua.h>

#include "constants.h"

#include <lualib.h>
#include <lauxlib.h>

typedef struct ecs_lua_system
{
    lua_State *L;
    ecs_entity_t e;
    int func_ref;
    const char *name;
    const char *module;
}ecs_lua_system;

typedef struct EcsLuaModule
{
    int x;
}EcsLuaModule;

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
    ecs_lua_system *sys = it->param;
    ecs_world_t *w = it->world;
    lua_State *L = sys->L;

    ecs_entity_t e = 0;
    ecs_entity_t type_entity = 0;
    int nargs = it->column_count;
    int idx = ecs_get_thread_index(w);

    ecs_time_t time;

    ecs_os_dbg("Lua system: \"%s\", columns: %d, func ref %d", ecs_get_name(w, it->system), nargs, sys->func_ref);

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

static int new_system(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);

    int func_ref = 0;
    const char *name = NULL;
    ecs_entity_t phase = 0;
    const char *signature = NULL;

    ecs_entity_t e = ecs_new(w, 0);
    ecs_entity_t t = e;

    lua_pushvalue(L, 1); /* luaL_ref() pops from the stack */
    func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    luaL_checktype(L, 1, LUA_TFUNCTION);

    name = luaL_checkstring(L, 2);
    if(lua_gettop(L) >= 3) phase = luaL_checkinteger(L, 3);
    if(lua_gettop(L) == 4) signature = luaL_checkstring(L, 4);

    e = ecs_new_system(w, e, name, phase, signature, entry_point);

    if(e != t)
    {
        ecs_os_dbg("ecs_lua: duplicate system ignored");
        luaL_unref(L, LUA_REGISTRYINDEX, func_ref);
        ecs_delete(w, t);
        lua_pushinteger(L, e);
        return 1;
    }

    ecs_lua_system *sys = lua_newuserdata(L, sizeof(ecs_lua_system));
    luaL_ref(L, LUA_REGISTRYINDEX);

    sys->func_ref = func_ref;
    sys->L = L;
    sys->e = e;

    ecs_set(w, e, EcsName, {.alloc_value = (char*)name});
    ecs_set(w, e, EcsContext, { sys });

    lua_pushinteger(L, e);

    return 1;
}

static int new_module(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);
    ecs_entity_t e = 0;

    const char *name = luaL_checkstring(L, 1);

    ecs_new_module(w, e, name, 4, 4);

    e = ecs_set(w, e, EcsName, {.alloc_value = (char*)name});

    lua_pushinteger(L, e);

    return 1;
}

void import_func(ecs_world_t *w)
{
    lua_State *L = ecs_get_context(w);

    ecs_os_dbg("ecs_lua: import callback");
    lua_pcall(L, 0, 0, 0);
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
        ecs_set_context(w, L);

        EcsLuaModule m;
        e = ecs_import(w, import_func, name, &m, sizeof(EcsLuaModule));

        ecs_set_context(w, orig);

        ecs_set(w, e, EcsName, {.alloc_value = (char*)name});
    }


    luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
    lua_getfield(L, -1, name);

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
    { "has", entity_has },
    { "add", add_type },
    { "remove", remove_type },
    { "clear", clear_entity },
    { "array", new_array },
    { "struct", new_struct },
    { "alias", new_alias },

    { "system", new_system },
    { "module", new_module },
    { "import", import_module },
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

int ecs_lua_init(ecs_lua_ctx *ctx)
{
    if(ctx->world == NULL || ctx->L == NULL) return 1;

    lua_State *L = ctx->L;

    ecs_lua_ctx *lctx = lua_newuserdata(L, sizeof(ecs_lua_ctx));
    lua_setfield(L, LUA_REGISTRYINDEX, "ecs_lua");

    memcpy(lctx, ctx, sizeof(ecs_lua_ctx));

    luaL_requiref(L, "ecs", luaopen_ecs, 1);
    lua_pop(L, 1);

    return 0;
}

void ecs_lua_exit(lua_State *L)
{
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);
}