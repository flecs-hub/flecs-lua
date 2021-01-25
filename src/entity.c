#include "private.h"

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

int new_entity(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = 0;

    const char *name = NULL;
    const char *components = NULL;
    int args = lua_gettop(L);

    if(!args)
    {
        e = ecs_new_id(w);
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
        else /* name (string|nil), components */
        {
            name = checkname(L, 1);
            components = luaL_checkstring(L, 2);
        }
    }
    else if(args == 3) /* entity, name (string|nil), components */
    {
        e = luaL_checkinteger(L, 1);
        name = checkname(L, 2);
        components = luaL_checkstring(L, 3);
    }
    else return luaL_error(L, "too many arguments");

    if(e && name)
    {/* ecs.new(123, "name") is idempotent, components are ignored */
        const char *existing = ecs_get_name(w, e);

        if(existing)
        {
            if(!strcmp(existing, name))
            {
                lua_pushinteger(L, e);
                return 1;
            }

            return luaL_error(L, "entity redefined with different name");
        }
    }

    if(!e && name)
    {/* ecs.new("name") is idempontent, components are ignored */
        e = ecs_lookup(w, name);

        if(e)
        {
            lua_pushinteger(L, e);
            return 1;
        }
    }

    /* create an entity, the following functions will take the same id */
    if(!e && args) e = ecs_new_id(w);

    if(components) ecs_add_type(w, e, ecs_type_from_str(w, components));

    if(name) ecs_set(w, e, EcsName, {.alloc_value = (char*)name});

    lua_pushinteger(L, e);

    return 1;
}

int delete_entity(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

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
            lua_pop(L, 1);
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

int new_tag(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);

    ecs_entity_t e = ecs_lookup(w, name);

    if(!e) e = ecs_set(w, e, EcsName, {.alloc_value = (char*)name});

    lua_pushinteger(L, e);

    return 1;
}

int entity_name(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    const char *name = ecs_get_name(w, e);

    lua_pushstring(L, name);

    return 1;
}

int entity_symbol(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    const EcsName *name = ecs_get(w, e, EcsName);

    lua_pushstring(L, name->symbol);

    return 1;
}

int entity_fullpath(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    char *path = ecs_get_fullpath(w, e);

    lua_pushstring(L, path);

    ecs_os_free(path);

    return 1;
}

int lookup_entity(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);

    ecs_entity_t e = ecs_lookup(w, name);

    lua_pushinteger(L, e);

    return 1;
}

int lookup_child(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t parent = luaL_checkinteger(L, 1);
    const char *name = luaL_checkstring(L, 2);

    ecs_entity_t e = ecs_lookup_child(w, parent, name);

    lua_pushinteger(L, e);

    return 1;
}

int lookup_path(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t parent = luaL_checkinteger(L, 1);
    const char *path = luaL_checkstring(L, 2);
    const char *sep = luaL_optstring(L, 3, ".");
    const char *prefix = luaL_optstring(L, 4, NULL);

    ecs_entity_t e = ecs_lookup_path_w_sep(w, parent, path, sep, prefix);

    lua_pushinteger(L, e);

    return 1;
}

int lookup_fullpath(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);

    ecs_entity_t e = ecs_lookup_fullpath(w, name);

    lua_pushinteger(L, e);

    return 1;
}

int lookup_symbol(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);

    ecs_entity_t e = ecs_lookup_symbol(w, name);

    lua_pushinteger(L, e);

    return 1;
}

int use_alias(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    const char *name = luaL_checkstring(L, 2);

    ecs_use(w, e, name);

    return 0;
}

int entity_has(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    int b;

    if(lua_isinteger(L, 2))
    {
        ecs_entity_t to_check = luaL_checkinteger(L, 2);
        b = ecs_has_entity(w, e, to_check);
    }
    else
    {
        ecs_type_t type = checktype(L, 2);
        b = ecs_has_type(w, e, type);
    }

    lua_pushboolean(L, b);

    return 1;
}

int entity_owns(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_type_t type = ecs_get_type(w, e);
    int b;

    if(lua_isinteger(L, 2))
    {
        ecs_entity_t to_check = luaL_checkinteger(L, 2);
        b = ecs_type_owns_entity(w, type, to_check, true);
    }
    else
    {
        ecs_type_t to_check = checktype(L, 2);
        b = ecs_type_owns_type(w, type, to_check, true);
    }

    lua_pushboolean(L, b);

    return 1;
}

int has_role(lua_State *L)
{
    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t role = luaL_checkinteger(L, 2);

    if((e & ECS_ROLE_MASK) == role) lua_pushboolean(L, 1);
    else lua_pushboolean(L, 0);

    return 1;
}

int is_alive(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    int b = ecs_is_alive(w, e);

    lua_pushboolean(L, b);

    return 1;
}

int exists(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    int b = ecs_exists(w, e);

    lua_pushboolean(L, b);

    return 1;
}

int entity_add(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    if(lua_isinteger(L, 2))
    {
        ecs_entity_t to_add = luaL_checkinteger(L, 2);
        ecs_add_entity(w, e, to_add);
    }
    else
    {
        ecs_type_t type = checktype(L, 2);
        ecs_add_type(w, e, type);
    }

    return 0;
}

int entity_remove(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    if(lua_isinteger(L, 2))
    {
        ecs_entity_t to_remove = luaL_checkinteger(L, 2);
        ecs_remove_entity(w, e, to_remove);
    }
    else
    {
        ecs_type_t type = checktype(L, 2);
        ecs_remove_type(w, e, type);
    }

    return 0;
}

int clear_entity(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = lua_tointeger(L, 1);

    ecs_clear(w, e);

    return 0;
}

int enable_entity(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = lua_tointeger(L, 1);

    if(lua_gettop(L) > 1)
    {
        ecs_entity_t c = lua_tointeger(L, 2);
        ecs_enable_component_w_entity(w, e, c, true);
    }
    else ecs_enable(w, e, true);

    return 0;
}

int disable_entity(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = lua_tointeger(L, 1);

    if(lua_gettop(L) > 1)
    {
        ecs_entity_t c = lua_tointeger(L, 2);
        ecs_enable_component_w_entity(w, e, c, false);
    }
    else ecs_enable(w, e, false);

    return 0;
}

int delete_children(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t parent = lua_tointeger(L, 1);

    ecs_delete_children(w, parent);

    return 0;
}

int new_type(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *expr = luaL_checkstring(L, 2);

    ecs_entity_t e = ecs_new_type(w, 0, name, expr);

    lua_pushinteger(L, e);

    return 1;
}

int get_type(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    ecs_type_t type = ecs_get_type(w, e);

    if(type)
    {
        void *ptr = lua_newuserdata(L, sizeof(ecs_type_t*));
        memcpy(ptr, &type, sizeof(ecs_type_t*));

        luaL_setmetatable(L, "ecs_type_t");
    }
    else lua_pushnil(L);

    return 1;
}

int get_typeid(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    ecs_entity_t typeid = ecs_get_typeid(w, e);

    lua_pushinteger(L, typeid);

    return 1;
}

int get_parent(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = 0;

    if(lua_gettop(L) == 2) c = luaL_checkinteger(L, 2);

    ecs_entity_t parent = ecs_get_parent_w_entity(w, e, c);

    lua_pushinteger(L, parent);

    return 1;
}

int enable_component(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);

    ecs_enable_component_w_entity(w, e, c, true);

    return 0;
}

int disable_component(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);

    ecs_enable_component_w_entity(w, e, c, false);

    return 0;
}

int is_component_enabled(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);

    int b = ecs_is_component_enabled_w_entity(w, e, c);

    lua_pushboolean(L, b);

    return 1;
}

int add_trait(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);
    ecs_entity_t t = luaL_checkinteger(L, 3);

    ecs_add_trait(w, e, c, t);

    return 0;
}

int remove_trait(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);
    ecs_entity_t t = luaL_checkinteger(L, 3);

    ecs_remove_trait(w, e, c, t);

    return 0;
}

int has_trait(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);
    ecs_entity_t t = luaL_checkinteger(L, 3);

    int b = ecs_has_trait(w, e, c, t);

    lua_pushboolean(L, b);

    return 1;
}

int set_trait(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);
    ecs_entity_t t = luaL_checkinteger(L, 3);

    void *ptr = ecs_get_mut_w_entity(w, e, ecs_trait(c, t), NULL);

    ecs_lua_to_ptr(w, L, 4, t, ptr);

    ecs_modified_w_entity(w, e, ecs_trait(c, t));

    return 0;
}

int set_trait_tag(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t t = luaL_checkinteger(L, 2);
    ecs_entity_t c = luaL_checkinteger(L, 3);

    void *ptr = ecs_get_mut_w_entity(w, e, ecs_trait(c, t), NULL);

    ecs_lua_to_ptr(w, L, 4, t, ptr);

    ecs_modified_w_entity(w, e, ecs_trait(c, t));

    return 0;
}

int get_trait(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);
    ecs_entity_t t = luaL_checkinteger(L, 3);

    const void *ptr = ecs_get_w_entity(w, e, ecs_trait(c, t));

    if(ptr) ecs_ptr_to_lua(w, L, t, ptr);
    else lua_pushnil(L);

    return 1;
}

int get_trait_tag(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t t = luaL_checkinteger(L, 2);
    ecs_entity_t c = luaL_checkinteger(L, 3);

    const void *ptr = ecs_get_w_entity(w, e, ecs_trait(c, t));

    if(ptr) ecs_ptr_to_lua(w, L, t, ptr);
    else lua_pushnil(L);

    return 1;
}

int add_instanceof(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t base = luaL_checkinteger(L, 2);

    ecs_add_entity(w, entity, ECS_INSTANCEOF | base);

    return 0;
}

int remove_instanceof(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t base = luaL_checkinteger(L, 2);

    ecs_remove_entity(w, entity, ECS_INSTANCEOF | base);

    return 0;
}

int add_childof(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t parent = luaL_checkinteger(L, 2);

    ecs_add_entity(w, entity, ECS_CHILDOF | parent);

    return 0;
}

int remove_childof(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t parent = luaL_checkinteger(L, 2);

    ecs_remove_entity(w, entity, ECS_CHILDOF | parent);

    return 0;
}

int add_owned(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    ecs_add_entity(w, entity, ECS_OWNED | component);

    return 0;
}

int add_switch(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t sw = luaL_checkinteger(L, 2);

    ecs_add_entity(w, e, ECS_SWITCH | sw);

    return 0;
}

int remove_switch(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t sw = luaL_checkinteger(L, 2);

    ecs_remove_entity(w, e, ECS_SWITCH | sw);

    return 0;
}

int get_case(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t sw = luaL_checkinteger(L, 2);

    ecs_entity_t cs = ecs_get_case(w, e, sw);

    lua_pushinteger(L, cs);

    return 1;
}

int add_case(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t sw_case = luaL_checkinteger(L, 2);

    ecs_add_entity(w, e, ECS_CASE | sw_case);

    return 0;
}

int remove_case(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t sw_case = luaL_checkinteger(L, 2);

    ecs_remove_entity(w, e, ECS_CASE | sw_case);

    return 0;
}

int new_enum(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *desc = luaL_checkstring(L, 2);

    ecs_entity_t ecs_typeid(EcsMetaType) = ecs_lookup_fullpath(w, "flecs.meta.MetaType");

    if(ecs_lookup_fullpath(w, name) || ecs_lookup(w, name)) luaL_argerror(L, 1, "component already exists");

    ecs_entity_t component = ecs_new_component_id(w);

    ecs_set(w, component, EcsName, {.alloc_value = (char*)name});

    ecs_set(w, component, EcsMetaType,
    {
        .kind = EcsEnumType,
        .size = sizeof(int),
        .alignment = ECS_ALIGNOF(int),
        .descriptor = desc
    });

    const EcsMetaType *meta = ecs_get(w, component, EcsMetaType);

    ecs_new_component(w, component, NULL, meta->size, meta->alignment);

    lua_pushinteger(L, component);

    return 1;
}

int new_bitmask(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *desc = luaL_checkstring(L, 2);

    ecs_entity_t ecs_typeid(EcsMetaType) = ecs_lookup_fullpath(w, "flecs.meta.MetaType");

    if(ecs_lookup_fullpath(w, name) || ecs_lookup(w, name)) luaL_argerror(L, 1, "component already exists");

    ecs_entity_t component = ecs_new_component_id(w);

    ecs_set(w, component, EcsName, {.alloc_value = (char*)name});

    ecs_set(w, component, EcsMetaType,
    {
        .kind = EcsBitmaskType,
        .size = sizeof(int),
        .alignment = ECS_ALIGNOF(int),
        .descriptor = desc
    });

    const EcsMetaType *meta = ecs_get(w, component, EcsMetaType);

    ecs_new_component(w, component, NULL, meta->size, meta->alignment);

    lua_pushinteger(L, component);

    return 1;
}

int new_array(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *element = luaL_checkstring(L, 2);
    lua_Integer count = luaL_checkinteger(L, 3);

    if(count < 0 || count > INT32_MAX) luaL_error(L, "element count out of range (%I)", count);

    if(ecs_lookup_fullpath(w, name) || ecs_lookup(w, name)) luaL_argerror(L, 1, "component already exists");

    ecs_strbuf_t buf = ECS_STRBUF_INIT;

    ecs_strbuf_append(&buf, "(%s,%lld)", element, count);

    char *desc = ecs_strbuf_get(&buf);

    ecs_entity_t ecs_typeid(EcsMetaType) = ecs_lookup_fullpath(w, "flecs.meta.MetaType");

    ecs_entity_t component = ecs_new_component_id(w);

    ecs_set(w, component, EcsName, {.alloc_value = (char*)name});

    ecs_set(w, component, EcsMetaType, {.kind = EcsArrayType, .descriptor = desc});

    const EcsMetaType *meta = ecs_get(w, component, EcsMetaType);

    ecs_new_component(w, component, NULL, meta->size, meta->alignment);

    ecs_os_free(desc);

    lua_pushinteger(L, component);

    return 1;
}

int new_struct(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *desc = luaL_checkstring(L, 2);

    ecs_entity_t ecs_typeid(EcsMetaType) = ecs_lookup_fullpath(w, "flecs.meta.MetaType");

    if(ecs_lookup_fullpath(w, name) || ecs_lookup(w, name)) luaL_argerror(L, 1, "component already exists");

    ecs_entity_t component = ecs_new_component_id(w);

    ecs_set(w, component, EcsName, {.alloc_value = (char*)name});

    ecs_set(w, component, EcsMetaType, {.kind = EcsStructType, .descriptor = desc});

    const EcsMetaType *meta = ecs_get(w, component, EcsMetaType);

    ecs_new_component(w, component, NULL, meta->size, meta->alignment);

    lua_pushinteger(L, component);

    return 1;
}

int new_alias(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *alias = luaL_checkstring(L, 2);

    ecs_entity_t ecs_typeid(EcsMetaType) = ecs_lookup_fullpath(w, "flecs.meta.MetaType");

    ecs_entity_t type_entity = ecs_lookup_fullpath(w, name);

    if(!type_entity) return luaL_argerror(L, 1, "component does not exist");

    if(!ecs_has(w, type_entity, EcsComponent)) return luaL_argerror(L, 1, "not a component");

    const EcsMetaType *meta = ecs_get(w, type_entity, EcsMetaType);

    if(!meta) return luaL_argerror(L, 1, "missing descriptor");

    if(ecs_lookup_fullpath(w, alias) || ecs_lookup(w, alias)) return luaL_argerror(L, 2, "alias already exists");

    ecs_entity_t component = ecs_new_component_id(w);

    ecs_set(w, component, EcsName, {.alloc_value = (char*)alias});

    ecs_set(w, component, EcsMetaType, {.kind = meta->kind, .descriptor = meta->descriptor});

    ecs_new_component(w, component, NULL, meta->size, meta->alignment);

    lua_pushinteger(L, component);

    return 1;
}

int get_func(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    const void *ptr = ecs_get_w_entity(w, e, component);

    if(ptr) ecs_ptr_to_lua(w, L, component, ptr);
    else lua_pushnil(L);

    return 1;
}

static int get_mutable(ecs_world_t *w, lua_State *L, ecs_entity_t e, ecs_entity_t component)
{
    bool is_added = 0;
    void *ptr = ecs_get_mut_w_entity(w, e, component, &is_added);

    if(ptr) ecs_ptr_to_lua(w, L, component, ptr);
    else lua_pushnil(L);

    lua_pushboolean(L, (int)is_added);

    return 2;
}

int get_mut(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    return get_mutable(w, L, e, component);
}

int patch_func(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);
    luaL_checktype(L, 3, LUA_TTABLE);

    bool is_added = 0;
    void *ptr = ecs_get_mut_w_entity(w, e, component, &is_added);

    ecs_lua_to_ptr(w, L, 3, component, ptr);

    ecs_modified_w_entity(w, e, component);

    lua_pushboolean(L, is_added);

    return 1;
}

int set_func(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    void *ptr = ecs_get_mut_w_entity(w, e, component, NULL);

    ecs_lua_to_ptr(w, L, 3, component, ptr);

    ecs_modified_w_entity(w, e, component);

    lua_pushinteger(L, e);

    return 1;
}

int new_ref(lua_State *L)
{
    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);

    ecs_ref_t *ref = lua_newuserdata(L, sizeof(ecs_ref_t));
    luaL_setmetatable(L, "ecs_ref_t");

    *ref = (ecs_ref_t){ .entity = e, .component = c };

    return 1;
}

int get_ref(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_ref_t *ref = luaL_checkudata(L, 1, "ecs_ref_t");
    ecs_entity_t entity = luaL_optinteger(L, 2, 0);
    ecs_entity_t component = 0;

    if(entity) component = luaL_checkinteger(L, 3);

    ecs_lua_assert(L, !entity || !ref->entity || entity == ref->entity, NULL);
    ecs_lua_assert(L, !component || !ref->component || component == ref->component, NULL);

    const void *ptr = ecs_get_ref_w_entity(w, ref, entity, component);

    ecs_ptr_to_lua(w, L, ref->component, ptr);

    return 1;
}

int singleton_get(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t component = luaL_checkinteger(L, 1);

    const void *ptr = ecs_get_w_entity(w, component, component);

    if(ptr) ecs_ptr_to_lua(w, L, component, ptr);
    else lua_pushnil(L);

    return 1;
}

int singleton_patch(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    bool is_added = 0;
    void *ptr = ecs_get_mut_w_entity(w, e, e, &is_added);

    ecs_lua_to_ptr(w, L, 2, e, ptr);

    ecs_modified_w_entity(w, e, e);

    lua_pushboolean(L, is_added);

    return 1;
}

int singleton_set(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t component = luaL_checkinteger(L, 1);

    void *ptr = ecs_get_mut_w_entity(w, component, component, NULL);

    ecs_lua_to_ptr(w, L, 2, component, ptr);

    ecs_modified_w_entity(w, component, component);

    return 1;
}

int new_prefab(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = 0;

    int args = lua_gettop(L);

    if(!args)
    {
        e = ecs_new_id(w);
        ecs_add_entity(w, e, EcsPrefab);
    }
    else if(args <= 2)
    {
        const char *id = luaL_checkstring(L, 1);
        const char *sig = luaL_optstring(L, 2, NULL);
        e = ecs_new_prefab(w, 0, id, sig);
        ecs_set(w, e, EcsName, { .alloc_value = (char*)id });
    }
    else return luaL_argerror(L, args, "too many arguments");

    lua_pushinteger(L, e);

    return 1;
}