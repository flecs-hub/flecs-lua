#include "private.h"

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

    if(e && ecs_is_alive(w, e) && name)
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

    if(e && !ecs_is_alive(w, e)) ecs_ensure(w, e);

    ecs_entity_t scope = ecs_get_scope(w);
    if(scope) ecs_add_pair(w, e, EcsChildOf, scope);

    if(components)
    {
        ecs_entity_desc_t desc =
        {
            .id = e,
            .add_expr = components,
            //XXX do we have to add it?
            //.add = { scope ? ecs_pair(EcsChildOf, scope) : 0 },
        };

        e = ecs_entity_init(w, &desc);
    }

    if(name) ecs_set_name(w, e, name);

    lua_pushinteger(L, e);

    return 1;
}

int new_id(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t id = ecs_new_id(w);

    lua_pushinteger(L, id);

    return 1;
}

int delete_entity(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity;

    if(lua_isinteger(L, 1))
    {
        entity = luaL_checkinteger(L, 1);
        ecs_delete(w, entity);
    }
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

    return 0;
}

int new_tag(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);

    ecs_entity_t e = ecs_lookup(w, name);

    if(!e) e = ecs_set_name(w, e, name);

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

int set_name(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    const char *name = lua_isnoneornil(L, 2) ? NULL : luaL_checkstring(L, 2);

    e = ecs_set_name(w, e, name);

    lua_pushinteger(L, e);

    return 1;
}

int entity_symbol(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    const char *symbol = ecs_get_symbol(w, e);

    if(!symbol) return 0;

    lua_pushstring(L, symbol);

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

    ecs_entity_t e = ecs_lookup_path_w_sep(w, parent, path, sep, prefix, false);

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

    ecs_entity_t e = ecs_lookup_symbol(w, name, true);

    lua_pushinteger(L, e);

    return 1;
}

int use_alias(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    const char *name = luaL_checkstring(L, 2);

    ecs_set_alias(w, e, name);

    return 0;
}

int entity_has(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    int args = lua_gettop(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    int b;

    if(args == 3)
    {
        ecs_entity_t relation = luaL_checkinteger(L, 2);
        ecs_entity_t object = luaL_checkinteger(L, 3);
        b = ecs_has_pair(w, e, relation, object);
    }
    else
    {
        ecs_entity_t to_check = luaL_checkinteger(L, 2);
        b = ecs_has_id(w, e, to_check);
    }

    lua_pushboolean(L, b);

    return 1;
}

int entity_owns(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t id = luaL_checkinteger(L, 2);

    int b = ecs_owns_id(w, e, id);

    lua_pushboolean(L, b);

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

int is_valid(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    int b = ecs_is_valid(w, e);

    lua_pushboolean(L, b);

    return 1;
}

int get_alive(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    ecs_entity_t id = ecs_get_alive(w, e);

    lua_pushinteger(L, id);

    return 1;
}

int ensure(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    ecs_ensure(w, e);

    return 0;
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

    int args = lua_gettop(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    if(args == 3)
    {
        ecs_entity_t relation = luaL_checkinteger(L, 2);
        ecs_entity_t object = luaL_checkinteger(L, 3);
        ecs_add_pair(w, e, relation, object);
    }
    else /* add(e, integer) */
    {
        ecs_entity_t to_add = luaL_checkinteger(L, 2);
        ecs_add_id(w, e, to_add);
    }

    return 0;
}

int entity_remove(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    int args = lua_gettop(L);
    ecs_entity_t e = luaL_checkinteger(L, 1);

    if(args == 3)
    {
        ecs_entity_t relation = luaL_checkinteger(L, 2);
        ecs_entity_t object = luaL_checkinteger(L, 3);
        ecs_remove_pair(w, e, relation, object);
    }
    else
    {
        ecs_entity_t to_remove = luaL_checkinteger(L, 2);
        ecs_remove_id(w, e, to_remove);
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
        ecs_enable_id(w, e, c, true);
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
        ecs_enable_id(w, e, c, false);
    }
    else ecs_enable(w, e, false);

    return 0;
}

int entity_count(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    int32_t count = ecs_count_id(w, e);

    lua_pushinteger(L, count);

    return 1;
}

int delete_children(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t parent = lua_tointeger(L, 1);

    ecs_delete_children(w, parent);

    return 0;
}

int get_type(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = 0;
    const ecs_type_t *type = NULL;
    int from_entity = 0;

    e = luaL_checkinteger(L, 1);
    from_entity = lua_toboolean(L, 2);

    if(from_entity) type = ecs_get_type(w, e);
    else type = ecs_get_type(w, e);

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

    ecs_entity_t parent = ecs_get_target(w, e, EcsChildOf, 0);

    lua_pushinteger(L, parent);

    return 1;
}

int enable_component(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);

    ecs_enable_id(w, e, c, true);

    return 0;
}

int disable_component(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);

    ecs_enable_id(w, e, c, false);

    return 0;
}

int is_component_enabled(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);

    int b = ecs_is_enabled_id(w, e, c);

    lua_pushboolean(L, b);

    return 1;
}

int add_pair(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);
    ecs_entity_t t = luaL_checkinteger(L, 3);

    ecs_add_id(w, e, ecs_pair(c, t));

    return 0;
}

int remove_pair(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);
    ecs_entity_t t = luaL_checkinteger(L, 3);

    ecs_remove_id(w, e, ecs_pair(c, t));

    return 0;
}

int has_pair(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);
    ecs_entity_t t = luaL_checkinteger(L, 3);

    int b = ecs_has_pair(w, e, c, t);

    lua_pushboolean(L, b);

    return 1;
}

int set_pair(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t relation = luaL_checkinteger(L, 2);
    ecs_entity_t object = luaL_checkinteger(L, 3);

    ecs_entity_t pair = ecs_pair(relation, object);

    if(!e)
    {
        e = ecs_new_id(w);
        ecs_entity_t scope = ecs_get_scope(w);
        if(scope) ecs_add_pair(w, e, EcsChildOf, scope);
    }

    void *ptr = ecs_get_mut_id(w, e, pair);

    ecs_lua_to_ptr(w, L, 4, relation, ptr);

    ecs_modified_id(w, e, pair);

    lua_pushinteger(L, e);

    return 1;
}

int set_pair_object(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t relation = luaL_checkinteger(L, 2);
    ecs_entity_t object = luaL_checkinteger(L, 3);

    ecs_entity_t pair = ecs_pair(relation, object);

    if(!e)
    {
        e = ecs_new_id(w);
        ecs_entity_t scope = ecs_get_scope(w);
        if(scope) ecs_add_pair(w, e, EcsChildOf, scope);
    }

    void *ptr = ecs_get_mut_id(w, e, pair);

    ecs_lua_to_ptr(w, L, 4, object, ptr);

    ecs_modified_id(w, e, pair);

    lua_pushinteger(L, e);

    return 0;
}

int get_pair(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t relation = luaL_checkinteger(L, 2);
    ecs_entity_t object = luaL_checkinteger(L, 3);

    ecs_entity_t pair = ecs_pair(relation, object);

    const void *ptr = ecs_get_id(w, e, pair);

    if(ptr) ecs_ptr_to_lua(w, L, relation, ptr);
    else lua_pushnil(L);

    return 1;
}

int get_mut_pair(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t relation = luaL_checkinteger(L, 2);
    ecs_entity_t object = luaL_checkinteger(L, 3);

    ecs_entity_t pair = ecs_pair(relation, object);

    void *ptr = ecs_get_mut_id(w, e, pair);

    if(ptr) ecs_ptr_to_lua(w, L, relation, ptr);
    else lua_pushnil(L);

    return 1;
}

int get_pair_object(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t relation = luaL_checkinteger(L, 2);
    ecs_entity_t object = luaL_checkinteger(L, 3);

    ecs_entity_t pair = ecs_pair(relation, object);

    const void *ptr = ecs_get_id(w, e, pair);

    if(ptr) ecs_ptr_to_lua(w, L, object, ptr);
    else lua_pushnil(L);

    return 1;
}

int get_mut_pair_object(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t relation = luaL_checkinteger(L, 2);
    ecs_entity_t object = luaL_checkinteger(L, 3);

    ecs_entity_t pair = ecs_pair(relation, object);

    void *ptr = ecs_get_mut_id(w, e, pair);

    if(ptr) ecs_ptr_to_lua(w, L, object, ptr);
    else lua_pushnil(L);

    return 1;
}

int make_pair(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t predicate = luaL_checkinteger(L, 1);
    ecs_entity_t object = luaL_checkinteger(L, 2);

    ecs_entity_t pair = ecs_pair(predicate, object);

    lua_pushinteger(L, pair);

    return 1;
}

int is_pair(lua_State *L)
{
    ecs_entity_t id = luaL_checkinteger(L, 1);

    int b = ECS_IS_PAIR(id);

    lua_pushboolean(L, b);

    return 1;
}

int pair_object(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t pair = luaL_checkinteger(L, 1);

    ecs_entity_t object = ecs_pair_object(w, pair);

    lua_pushinteger(L, object);

    return 1;
}

int add_instanceof(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t base = luaL_checkinteger(L, 2);

    ecs_add_pair(w, entity, EcsIsA, base);

    return 0;
}

int remove_instanceof(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t base = luaL_checkinteger(L, 2);

    ecs_remove_pair(w, entity, EcsIsA, base);

    return 0;
}

int add_childof(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t parent = luaL_checkinteger(L, 2);

    ecs_add_pair(w, entity, EcsChildOf, parent);

    return 0;
}

int remove_childof(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t parent = luaL_checkinteger(L, 2);

    ecs_remove_pair(w, entity, EcsChildOf, parent);

    return 0;
}

int entity_override(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t entity = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    ecs_add_id(w, entity, ECS_OVERRIDE | component);

    return 0;
}

static void init_scope(ecs_world_t *w, ecs_entity_t id)
{
    ecs_entity_t scope = ecs_get_scope(w);

    if(scope) ecs_add_pair(w, id, EcsChildOf, scope);
}

int new_enum(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *desc = luaL_checkstring(L, 2);

    if(ecs_lookup_fullpath(w, name) || ecs_lookup(w, name)) luaL_argerror(L, 1, "component already exists");

    ecs_entity_t component = ecs_entity_init(w, &(ecs_entity_desc_t){ .use_low_id = true });

    ecs_set_name(w, component, name);

    int err = ecs_meta_from_desc(w, component, EcsEnumType, desc);
    
    if(err) return luaL_argerror(L, 2, "invalid descriptor");

    init_scope(w, component);

    lua_pushinteger(L, component);

    return 1;
}

int new_bitmask(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *desc = luaL_checkstring(L, 2);

    if(ecs_lookup_fullpath(w, name) || ecs_lookup(w, name)) luaL_argerror(L, 1, "component already exists");

    ecs_entity_t component = ecs_entity_init(w, &(ecs_entity_desc_t){ .use_low_id = true });

    ecs_set_name(w, component, name);

    int err = ecs_meta_from_desc(w, component, EcsBitmaskType, desc);
    
    if(err) return luaL_argerror(L, 2, "invalid descriptor");

    init_scope(w, component);

    lua_pushinteger(L, component);

    return 1;
}

int new_array(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    lua_Integer element = luaL_checkinteger(L, 2);
    lua_Integer count = luaL_checkinteger(L, 3);

    if(count < 0 || count > INT32_MAX) luaL_error(L, "element count out of range (%I)", count);

    if(ecs_lookup_fullpath(w, name) || ecs_lookup(w, name)) luaL_argerror(L, 1, "component already exists");

    ecs_array_desc_t desc =
    {
        .type = element,
        .count = count
    };

    ecs_entity_t component = ecs_array_init(w, &desc);

    ecs_set_name(w, component, name);

    init_scope(w, component);

    lua_pushinteger(L, component);

    return 1;
}

int new_struct(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *desc = luaL_checkstring(L, 2);

    if(ecs_lookup_fullpath(w, name) || ecs_lookup(w, name)) luaL_argerror(L, 1, "component already exists");

    ecs_entity_t component = ecs_entity_init(w, &(ecs_entity_desc_t){ .use_low_id = true });

    ecs_set_name(w, component, name);

    int err = ecs_meta_from_desc(w, component, EcsStructType, desc);

    if(err) return luaL_argerror(L, 2, "invalid descriptor");

    ecs_set(w, component, EcsMetaType, {.kind = EcsStructType});

    init_scope(w, component);

    lua_pushinteger(L, component);

    return 1;
}

int new_alias(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *name = luaL_checkstring(L, 1);
    const char *alias = luaL_checkstring(L, 2);

    ecs_entity_t type_entity = ecs_lookup_fullpath(w, name);

    if(!type_entity) return luaL_argerror(L, 1, "component does not exist");

    if(!ecs_has(w, type_entity, EcsComponent)) return luaL_argerror(L, 1, "not a component");

    const EcsMetaType *meta = ecs_get(w, type_entity, EcsMetaType);

    if(!meta) return luaL_argerror(L, 1, "missing descriptor");

    if(ecs_lookup_fullpath(w, alias) || ecs_lookup(w, alias)) return luaL_argerror(L, 2, "alias already exists");

    ecs_entity_t component = ecs_entity_init(w, &(ecs_entity_desc_t){ .use_low_id = true });

    ecs_set_name(w, component, alias);

    /* XXX: copy components? */

    init_scope(w, component);

    lua_pushinteger(L, component);

    return 1;
}

int get_func(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    const void *ptr = ecs_get_id(w, e, component);

    if(ptr) ecs_ptr_to_lua(w, L, component, ptr);
    else lua_pushnil(L);

    return 1;
}

static int get_mutable(ecs_world_t *w, lua_State *L, ecs_entity_t e, ecs_entity_t component)
{
    void *ptr = ecs_get_mut_id(w, e, component);

    if(ptr) ecs_ptr_to_lua(w, L, component, ptr);
    else lua_pushnil(L);

    return 1;
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

    void *ptr = ecs_get_mut_id(w, e, component);

    ecs_lua_to_ptr(w, L, 3, component, ptr);

    return 0;
}

int set_func(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t component = luaL_checkinteger(L, 2);

    if(!e)
    {
        e = ecs_new_id(w);
        ecs_entity_t scope = ecs_get_scope(w);
        if(scope) ecs_add_pair(w, e, EcsChildOf, scope);
    }

    void *ptr = ecs_get_mut_id(w, e, component);

    ecs_lua_to_ptr(w, L, 3, component, ptr);

    ecs_modified_id(w, e, component);

    lua_pushinteger(L, e);

    return 1;
}

int new_ref(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L); // TODO: verify ref world vs api world
    ecs_entity_t e = luaL_checkinteger(L, 1);
    ecs_entity_t c = luaL_checkinteger(L, 2);

    ecs_ref_t *ref = lua_newuserdata(L, sizeof(ecs_ref_t));
    luaL_setmetatable(L, "ecs_ref_t");

    *ref = ecs_ref_init_id(w, e, c);

    return 1;
}

int get_ref(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_ref_t *ref = luaL_checkudata(L, 1, "ecs_ref_t");
    ecs_entity_t id = ref->id;

    if(lua_gettop(L)> 1) id = luaL_checkinteger(L, 2);

    //if(!id || !ref->id || id == ref->id) return luaL_argerror(L, )
    ecs_lua_assert(L, !id || !ref->id || id == ref->id, NULL);

    const void *ptr = ecs_ref_get_id(w, ref, id);

    ecs_ptr_to_lua(w, L, ref->id, ptr);

    return 1;
}

int singleton_get(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t component = luaL_checkinteger(L, 1);

    const void *ptr = ecs_get_id(w, component, component);

    if(ptr) ecs_ptr_to_lua(w, L, component, ptr);
    else lua_pushnil(L);

    return 1;
}

int singleton_patch(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    void *ptr = ecs_get_mut_id(w, e, e);

    ecs_lua_to_ptr(w, L, 2, e, ptr);

    ecs_modified_id(w, e, e);

    return 1;
}

int singleton_set(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t component = luaL_checkinteger(L, 1);

    void *ptr = ecs_get_mut_id(w, component, component);

    ecs_lua_to_ptr(w, L, 2, component, ptr);

    ecs_modified_id(w, component, component);

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
        ecs_entity_t scope = ecs_get_scope(w);
        if(scope) ecs_add_pair(w, e, EcsChildOf, scope);
        ecs_add_id(w, e, EcsPrefab);
    }
    else if(args <= 2)
    {
        const char *id = luaL_checkstring(L, 1);
        const char *sig = luaL_optstring(L, 2, NULL);

        e = ecs_entity_init(w, &(ecs_entity_desc_t)
        {
            .name = id,
            .add_expr = sig,
            .add = {EcsPrefab}
        });
    }
    else return luaL_argerror(L, args, "too many arguments");

    lua_pushinteger(L, e);

    return 1;
}