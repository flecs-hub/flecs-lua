#include "private.h"

int bulk_new(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t id = 0;
    lua_Integer count = 0;
    const ecs_entity_t *entities = NULL;

    int noreturn = 0;
    int args = lua_gettop(L);
    int last_type = lua_type(L, args);

    if(args == 2 && last_type == LUA_TBOOLEAN) /* bulk_new(count, noreturn) */
    {
        count = luaL_checkinteger(L, 1);
        noreturn = lua_toboolean(L, 2);
    }
    else if(args >= 2) /* bulk_new(component, count, [noreturn]) */
    {
        id = luaL_checkinteger(L, 1);

        count = luaL_checkinteger(L, 2);
        noreturn = lua_toboolean(L, 3);
    }
    else count = luaL_checkinteger(L, 1); /* bulk_new(count) */

    entities = ecs_bulk_new_w_id(w, id, count);

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
