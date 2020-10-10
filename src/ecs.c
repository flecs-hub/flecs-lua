#include "private.h"

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

static void system_entry_point(ecs_iter_t *it)
{
    ecs_world_t *w = it->world;
    ecs_lua_system *sys = it->param;
    lua_State *L = sys->L;

    ecs_lua__prolog(L);
    ecs_assert(!strcmp(sys->signature, ecs_get(w, it->system, EcsSignatureExpr)->expr), ECS_INTERNAL_ERROR, NULL);

    ecs_time_t time;
    int idx = ecs_get_thread_index(w);

    ecs_os_dbg("Lua system: \"%s\", %d columns, count %d, func ref %d",
        ecs_get_name(w, it->system), it->column_count, it->count, sys->func_ref);

    int type = lua_rawgeti(L, LUA_REGISTRYINDEX, sys->func_ref);

    luaL_checktype(L, -1, LUA_TFUNCTION);

    ecs_os_get_time(&time);

    ecs_iter_to_lua(it, L, NULL);

    print_time(&time, "iter serialization");

    lua_pushvalue(L, -1);
    int it_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    ecs_os_get_time(&time);

    int ret = lua_pcall(L, 1, 0, 0);

    print_time(&time, "system");

    if(ret) ecs_os_err("error running system \"%s\" (%d): %s", ecs_get_name(w, it->system), ret, lua_tostring(L, 1));

    ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);

    lua_rawgeti(L, LUA_REGISTRYINDEX, it_ref);

    ecs_assert(lua_type(L, -1) == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    ecs_os_get_time(&time);

    ecs_lua_to_iter(w, L, -1);

    print_time(&time, "iter deserialization");

    luaL_unref(L, LUA_REGISTRYINDEX, it_ref);
    lua_pop(L, 1);

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

static const char *checkname(lua_State *L, int arg)
{
    int type = lua_type(L, arg);

    if(type == LUA_TSTRING) return luaL_checkstring(L, arg);
    else if(type != LUA_TNIL) luaL_argerror(L, arg, "expected string or nil for name");

    return NULL;
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
    else if(args == 1) /* entity | name(string) */
    {
        int type = lua_type(L, 1);

        if(lua_isinteger(L, 1)) e = luaL_checkinteger(L, 1);
        else if(type == LUA_TSTRING) name = luaL_checkstring(L, 1);
        else return luaL_argerror(L, 1, "expected entity or name");
    }
    else if(args == 2)
    {
        if(lua_isinteger(L, 1)) /* entity, name (string) */
        {
            luaL_checktype(L, 2, LUA_TSTRING);

            e = luaL_checkinteger(L, 1);
            name = luaL_checkstring(L, 2);
        }
        else /* name (string|nil), component */
        {
            name = checkname(L, 1);
            components = luaL_checkstring(L, 2);
        }
    }
    else if(args == 3) /* entity, name (string|nil), component */
    {
        e = luaL_checkinteger(L, 1);
        name = checkname(L, 2);
        components = luaL_checkstring(L, 3);
    }
    else return luaL_error(L, "too many arguments");

    if(args) e = ecs_new_entity(w, e, name, components);

    if(name) ecs_set(w, e, EcsName, {.alloc_value = (char*)name});

    lua_pushinteger(L, e);

    return 1;
}

static int bulk_new(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    lua_Integer count = 0;
    ecs_type_t type = NULL;
    const ecs_entity_t* entities = NULL;

    if(lua_gettop(L) == 2)
    {
        ecs_entity_t type_entity = 0;

        if(lua_isinteger(L, 1)) type_entity = luaL_checkinteger(L, 1);
        else
        {
            const char *name = luaL_checkstring(L, 1);

            type_entity = ecs_lookup(w, name);

            if(!type_entity) return luaL_argerror(L, 2, "could not find type");
        }

        type = ecs_type_from_entity(w, type_entity);

        count = luaL_checkinteger(L, 2);
    }
    else count = luaL_checkinteger(L, 1);

    entities = ecs_bulk_new_w_type(w, type, count);

    lua_newtable(L);

    lua_Integer i;
    for(i=0; i < count; i++)
    {
        lua_pushinteger(L, entities[i]);
        lua_rawseti(L, -2, i+1);
    }

    return 1;
}

static int delete_entity(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t entity;

    if(lua_isinteger(L, 1)) entity = luaL_checkinteger(L, 1);
    else if(lua_type(L, 1) == LUA_TTABLE)
    {
        int len = lua_rawlen(L, 1);
        int i;
        for(i=0; i < len; i++)
        {
            lua_rawgeti(L, 1, i + 1);
            entity = luaL_checkinteger(L, -1);
            ecs_delete(w, entity);
        }

        return 0;
    }
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
    ecs_entity_t to_check = 0;

    if(lua_isinteger(L, 2)) to_check = luaL_checkinteger(L, 2);
    else
    {
        const char *name = luaL_checkstring(L, 2);
        to_check = ecs_lookup_fullpath(w, name);

        if(!to_check) return luaL_argerror(L, 2, "could not find type");
    }

    if(ecs_has_entity(w, e, to_check)) lua_pushboolean(L, 1);
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

static int is_alive(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    if(ecs_is_alive(w, e)) lua_pushboolean(L, 1);
    else lua_pushboolean(L, 0);

    return 1;
}

static int exists(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    if(ecs_exists(w, e)) lua_pushboolean(L, 1);
    else lua_pushboolean(L, 0);

    return 1;
}

static int add_type(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t entity_add = 0;

    if(lua_isinteger(L, 2)) entity_add = luaL_checkinteger(L, 2);
    else
    {
        const char *name = luaL_checkstring(L, 2);
        entity_add = ecs_lookup_fullpath(w, name);

        if(!entity_add) return luaL_argerror(L, 2, "could not find type");
    }

    ecs_add_entity(w, e, entity_add);

    return 0;
}

static int remove_type(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t to_remove = 0;

    if(lua_isinteger(L, 2)) to_remove = luaL_checkinteger(L, 2);
    else
    {
        const char *name = luaL_checkstring(L, 2);
        to_remove = ecs_lookup_fullpath(w, name);

        if(!to_remove) return luaL_argerror(L, 2, "could not find type");
    }

    ecs_remove_entity(w, e, to_remove);

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
    const char *element = luaL_checkstring(L, 2);
    lua_Integer count = luaL_checkinteger(L, 3);

    if(count < 0 || count > INT32_MAX) luaL_error(L, "element count out of range (%I)", count);

    ecs_strbuf_t buf = ECS_STRBUF_INIT;

    ecs_strbuf_append(&buf, "(%s,%lld)", element, count);

    char *desc = ecs_strbuf_get(&buf);

    ecs_entity_t ecs_entity(EcsMetaType) = ecs_lookup_fullpath(w, "flecs.meta.MetaType");

    ecs_entity_t component = ecs_set(w, 0, EcsName, {.alloc_value = (char*)name});

    ecs_set(w, component, EcsMetaType, {.kind = EcsArrayType, .descriptor = desc});

    const EcsMetaType *meta = ecs_get(w, component, EcsMetaType);

    ecs_new_component(w, component, NULL, meta->size, meta->alignment);

    ecs_os_free(desc);

    lua_pushinteger(L, component);

    return 1;
}

static int new_struct(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *desc = luaL_checkstring(L, 2);

    ecs_entity_t ecs_entity(EcsMetaType) = ecs_lookup_fullpath(w, "flecs.meta.MetaType");

    ecs_entity_t component = ecs_set(w, 0, EcsName, {.alloc_value = (char*)name});

    ecs_set(w, component, EcsMetaType, {.kind = EcsStructType, .descriptor = desc});

    const EcsMetaType *meta = ecs_get(w, component, EcsMetaType);

    ecs_new_component(w, component, NULL, meta->size, meta->alignment);

    lua_pushinteger(L, component);

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

    if(ptr) ecs_ptr_to_lua(w, L, component, ptr);
    else lua_pushnil(L);

    return 1;
}

static void pushmutable(
    ecs_world_t *w,
    lua_State *L,
    ecs_entity_t e,
    ecs_entity_t component,
    void *ptr)
{
    ecs_ptr_to_lua(w, L, component, ptr);

    lua_createtable(L, 0, 1);
    lua_createtable(L, 3, 0);

    lua_pushlightuserdata(L, ptr);
    lua_rawseti(L, -2, 1);

    lua_pushinteger(L, e);
    lua_rawseti(L, -2, 2);

    lua_pushinteger(L, component);
    lua_rawseti(L, -2, 3);

    lua_setfield(L, -2, "__ecs_mutable");
    lua_setmetatable(L, -2);
}

static get_mutable(ecs_world_t *w, lua_State *L, ecs_entity_t e, ecs_entity_t component)
{

    bool is_added = 0;
    void *ptr = ecs_get_mut_w_entity(w, e, component, &is_added);

    if(!ptr)
    {
        lua_pushnil(L);
        lua_pushboolean(L, 0);
        return 2;
    }

    pushmutable(w, L, e, component, ptr);

    lua_pushboolean(L, (int)is_added);

    return 2;
}

static int get_mut(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    return get_mutable(w, L, e, component);
}

static int mutable_modified(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    luaL_checktype(L, 1, LUA_TTABLE);

    luaL_getmetafield(L, 1, "__ecs_mutable");

    if(lua_type(L, -1) != LUA_TTABLE) return luaL_error(L, "table is not a mutable");

    lua_rawgeti(L, -1, 1);
    void *ptr = lua_touserdata(L, -1);
    lua_pop(L, 1);

    lua_rawgeti(L, -1, 2);
    ecs_entity_t e = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_rawgeti(L, -1, 3);
    ecs_entity_t c = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    ecs_lua_to_ptr(w, L, 1, c, ptr);

    ecs_modified_w_entity(w, e, c);

    return 0;
}

static int set_func(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    void *ptr = ecs_get_mut_w_entity(w, e, component, NULL);

    ecs_lua_to_ptr(w, L, 3, component, ptr);

    ecs_modified_w_entity(w, e, component);

    lua_pushinteger(L, e);

    return 1;
}

static int singleton_get(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t component = luaL_checkinteger(L, 1);

    const void *ptr = ecs_get_w_entity(w, component, component);

    if(ptr) ecs_ptr_to_lua(w, L, component, ptr);
    else lua_pushnil(L);

    return 1;
}

static int singleton_get_mut(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t component = luaL_checkinteger(L, 1);

    return get_mutable(w, L, component, component);
}

static int singleton_set(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t component = luaL_checkinteger(L, 1);

    void *ptr = ecs_get_mut_w_entity(w, component, component, NULL);

    ecs_lua_to_ptr(w, L, 2, component, ptr);

    ecs_modified_w_entity(w, component, component);

    return 1;
}

ecs_iter_t *ecs_lua__checkiter(lua_State *L, int idx)
{
    if(luaL_getmetafield(L, idx, "__ecs_iter") != LUA_TTABLE) luaL_error(L, "table is not an iterator");

    lua_rawgeti(L, -1, 1);
    ecs_iter_t *it = lua_touserdata(L, -1);
    lua_pop(L, 2);

    return it;
}

static ecs_iter_t *get_iter_columns(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_iter_t *it = ecs_lua__checkiter(L, 1);

    luaL_getsubtable(L, 1, "columns");

    return it;
}

static int column(lua_State *L)
{
    ecs_iter_t *it = get_iter_columns(L);

    lua_Integer i = luaL_checkinteger(L, 2);

    if(i < 1 || i > it->column_count) luaL_argerror(L, 2, "invalid column index");

    lua_rawgeti(L, -1, i);

    return 1;
}

static int columns(lua_State *L)
{
    ecs_iter_t *it = get_iter_columns(L);

    int i;
    for(i=1; i <= it->column_count; i++)
    {
        lua_rawgeti(L, 2, i);
    }

    return it->column_count;
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

    ecs_entity_t e = ecs_new_system(w, 0, name, phase, signature, system_entry_point);

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

static void import_entry_point(ecs_world_t *w)
{
    ecs_lua_module *m = ecs_get_context(w);
    ecs_lua_ctx *ctx = m->ctx;
    lua_State *L = ctx->L;

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

    ecs_import(w, import_entry_point, name, NULL, 0);

    ecs_set_context(w, orig);

    ecs_assert(!ctx->error, ECS_INTERNAL_ERROR, lua_tostring(L, -1));

    if(ctx->error) return lua_error(L);

    lua_pushinteger(L, m.e);

    ecs_lua__epilog(L);
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

static int get_time(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_time_t time = {0};

    ecs_os_get_time(&time);

    pushtime(L, &time);

    return 1;
}

static int time_measure(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_time_t start = checktime(L, 1);

    double time = ecs_time_measure(&start);

    lua_pushnumber(L, time);

    return 1;
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
    ecs_world_t *w = ecs_lua_get_world(L);

    lua_Number delta_time = luaL_checknumber(L, 1);

    int b = ecs_progress(w, delta_time);

    lua_pushboolean(L, b); 

    return 1;
}

static int progress_cb(lua_State *L)
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

    ecs_ptr_to_lua(w, L, e, &world_info);

    return 1;
}

static int lquit(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_quit(w);

    return 0;
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
    { "is_alive", is_alive },
    { "exists", exists },
    { "add", add_type },
    { "remove", remove_type },
    { "clear", clear_entity },
    { "array", new_array },
    { "struct", new_struct },
    { "alias", new_alias },

    { "get", get_func },
    { "get_mut", get_mut },
    { "modified", mutable_modified },
    { "set", set_func },

    { "singleton_get", singleton_get },
    { "singleton_get_mut", singleton_get_mut },
    { "singleton_modified", mutable_modified },
    { "singleton_set", singleton_set },

    { "column", column },
    { "columns", columns },

    { "system", new_system },
    { "module", new_module },

    { "log", print_log },
    { "err", print_err },
    { "dbg", print_dbg },
    { "warn", print_warn },
    { "assert", assert_func },

    { "get_time", get_time },
    { "time_measure", time_measure },

    { "set_target_fps", set_target_fps },
    { "progress", progress },
    { "progress_cb", progress_cb },
    { "world_info", world_info },
    { "quit", lquit },

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

void ecs_lua_exit(lua_State *L)
{
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);

    if( !(ctx->internal & ECS_LUA__KEEPOPEN) ) lua_close(L);
}

int ecs_lua_set_state(ecs_world_t *w, lua_State *L)
{
    ecs_entity_t ecs_entity(EcsLuaHost) = ecs_lookup_fullpath(w, "flecs.lua.LuaHost");
    ecs_assert(ecs_entity(EcsLuaHost) != 0, ECS_INTERNAL_ERROR, NULL);

    EcsLuaHost *host = ecs_singleton_get_mut(w, EcsLuaHost);

    ecs_lua_exit(host->L);

    ecs_lua_ctx param = { .L = L, .world = w, .internal = ECS_LUA__KEEPOPEN };

    host->L = L;
    host->ctx = ctx_init(param);

    ecs_singleton_modified(w, EcsLuaHost);

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

ECS_DTOR(EcsLuaHost, ptr,
{
    ecs_lua_exit(ptr->L);
    ptr->L = NULL;
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

    ecs_singleton_set(w, EcsLuaHost, { .L = L, .ctx = ctx });

    ecs_set_component_actions(w, EcsLuaHost,
    {
        .dtor = ecs_dtor(EcsLuaHost)
    });
}
