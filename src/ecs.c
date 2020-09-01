#include <flecs_lua.h>

#include "constants.h"

#include <lualib.h>
#include <lauxlib.h>

int luaopen_ecs(lua_State *L);

typedef struct ecs_lua_system
{
    lua_State *L;
    const char *name;
    const char *signature;
}ecs_lua_system;

ecs_lua_ctx *ecs_lua_get_context(lua_State *L)
{
    lua_pushstring(L, "ecs_lua");
    lua_gettable(L, LUA_REGISTRYINDEX);
    ecs_lua_ctx *p = lua_touserdata(L, -1);
    lua_pop(L, 1);
    return p;
}

ecs_world_t *ecs_lua_get_world(lua_State *L)
{
    ecs_lua_ctx *p = ecs_lua_get_context(L);
    return p->world;
}

static void entry_point(ecs_iter_t *it)
{
    ecs_lua_system *sys = it->param;
    ecs_world_t *w = it->world;
    lua_State *L = sys->L;
    int nargs = it->column_count;

    //TODO: stuff

    lua_pushstring(L, sys->name);
    lua_pcall(L, nargs, 0, 0);
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
    else luaL_error(L, "too many arguments");

    if(name)
    {
        e = ecs_new_entity(w, e, NULL, components);
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

        if(!type_entity) luaL_argerror(L, 2, "could not find type");

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

        if(!entity) luaL_argerror(L, 1, "could not find entity");
    }

    ecs_delete(w, entity);

    return 0;
}

static int new_tag(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    const char *name = luaL_checkstring(L, 1);

    ecs_entity_t e = ecs_new(w, 0);

    //const char *e_name = ecs_name_from_symbol(w, name);

    ecs_set(w, e, EcsName, {.alloc_value = (char*)name, .symbol = name});

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

        if(!type_entity) luaL_argerror(L, 2, "could not find type");
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

        if(!type_entity) luaL_argerror(L, 2, "could not find type");
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

        if(!type_entity) luaL_argerror(L, 2, "could not find type");
    }

    ecs_type_t type = ecs_type_from_entity(w, type_entity);

    ecs_remove_type(w, e, type);

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

static int new_system(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);

    const char *name = luaL_checkstring(L, 1);
    ecs_entity_t phase = 0;
    const char *signature = NULL;

    if(lua_gettop(L) == 3) signature = luaL_checkstring(L, 3);

    phase = luaL_checkinteger(L, 2);

    lua_State *S = ctx->new_state_cb();
    if(S = NULL) return luaL_error(L, "failed to create system");

    lua_pushstring(S, "ecs_lua");
    lua_pushlightuserdata(S, ctx);
    lua_settable(S, LUA_REGISTRYINDEX);
#if 0
    lua_pushstring(L, "ecs_lua_systems");
    lua_gettable(L, LUA_REGISTRYINDEX);
    int len = lua_objlen(L, );
    lua_pushinteger(L, len);
#endif

    lua_pushstring(S, "ecs_lua_system");
    ecs_lua_system *sys = lua_newuserdata(S, sizeof(ecs_lua_system));
    lua_settable(S, LUA_REGISTRYINDEX);

    //TODO: duplicate signature string

    luaL_requiref(S, "ecs", luaopen_ecs, 1);
    lua_pop(S, 1);

    ecs_entity_t e = 0;

    e = ecs_new_system(w, e, NULL, phase, signature, entry_point);

    ecs_set(w, e, EcsName, {.alloc_value = (char*)name});
    ecs_set(w, e, EcsContext, {sys});

    sys->L = S;
    sys->name = ecs_get_name(w, e);
    // sys->signature =

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
    { "array", new_array },
    { "struct", new_struct },

    { "system", new_system },
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
    if(ctx->new_state_cb == NULL || ctx->close_state_cb == NULL) return 1;

    lua_State *L = ctx->L;

    lua_pushstring(ctx->L, "ecs_lua");
    ecs_lua_ctx *lctx = lua_newuserdata(ctx->L, sizeof(ecs_lua_ctx));
    lua_settable(L, LUA_REGISTRYINDEX);

    memcpy(lctx, ctx, sizeof(ecs_lua_ctx));

    lua_pushstring(L, "ecs_lua_systems");
    lua_newtable(L);
    lua_settable(L, LUA_REGISTRYINDEX);

    //luaL_requiref(L, "_G", luaopen_ecs, 1);
    luaL_requiref(L, "ecs", luaopen_ecs, 1);
    lua_pop(L, 1);

    return 0;
}

int ecs_lua_exit(lua_State *L)
{
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);

#if 0
    lua_pushstring(L, "ecs_lua_systems");
    lua_gettable(L, LUA_REGISTRYINDEX);
    int systems = lua_objlen(L, -1);
    printf("TEST: freeing %d systems\n", systems);
    int i;
    for(i=0; i < systems; i++)
    {
        lua_rawgeti(L, -2, i+1);
        ecs_lua_system *S = lua_touserdata(L, -2);
        ctx->close_state_cb(S->L);
    }

    lua_pop(L, 1);
#endif

    return 0;
}