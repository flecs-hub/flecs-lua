#include "private.h"

int bulk_new(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    int noreturn = 0;
    int args = lua_gettop(L);
    int last_type = lua_type(L, args);
    lua_Integer count = 0;
    ecs_type_t type = NULL;
    const ecs_entity_t *entities = NULL;

    if(args == 2 && last_type == LUA_TBOOLEAN) /* bulk_new(count, noreturn) */
    {
        count = luaL_checkinteger(L, 1);
        noreturn = lua_toboolean(L, 2);
    }
    else if(args >= 2) /* bulk_new(component, count, [noreturn]) */
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
        noreturn = lua_toboolean(L, 3);
    }
    else count = luaL_checkinteger(L, 1);

    entities = ecs_bulk_new_w_type(w, type, count);

    if(noreturn) return 0;

    lua_createtable(L, count, 0);

    lua_Integer i;
    for(i=0; i < count; i++)
    {
        lua_pushinteger(L, entities[i]);
        lua_rawseti(L, -2, i+1);
    }

    return 1;
}

int bulk_delete(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
    ecs_filter_t filter = checkfilter(L, 1);

    ecs_bulk_delete(w, &filter);

    return 0;
}