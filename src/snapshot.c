#include "private.h"

static ecs_snapshot_t *checksnapshot(lua_State *L, int arg)
{
    ecs_snapshot_t **snapshot = luaL_checkudata(L, arg, "ecs_snapshot_t");

    if(!*snapshot) luaL_argerror(L, arg, "snapshot was collected");

    return *snapshot;
}

int snapshot_gc(lua_State *L)
{
    ecs_snapshot_t **ptr = luaL_checkudata(L, 1, "ecs_snapshot_t");
    ecs_snapshot_t *snapshot = *ptr;

    if(snapshot) ecs_snapshot_free(snapshot);

    *ptr = NULL;

    return 0;
}

int snapshot_take(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_iter_t *it;
    ecs_snapshot_t *snapshot;

    if(lua_gettop(L) > 0)
    {
        it = ecs_lua__checkiter(L, 1);
        ecs_iter_next_action_t next_action = ecs_filter_next;
        snapshot = ecs_snapshot_take_w_iter(it, next_action);
    }
    else snapshot = ecs_snapshot_take(w);

    ecs_snapshot_t **ptr = lua_newuserdata(L, sizeof(ecs_snapshot_t*));
    *ptr = snapshot;

    /* Tie object to world */
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_setuservalue(L, -2);

    luaL_setmetatable(L, "ecs_snapshot_t");
    register_collectible(L, w, -1);

    return 1;
}

int snapshot_restore(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
    ecs_snapshot_t *snapshot = checksnapshot(L, 1);

    ecs_lua_check_world(L, w, 1);

    ecs_snapshot_restore(w, snapshot);

    /* Snapshot data is moved into world and is considered freed */
    ecs_snapshot_t **ptr = lua_touserdata(L, 1);
    *ptr = NULL;

    return 0;
}

int snapshot_iter(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
    ecs_snapshot_t *snapshot = checksnapshot(L, 1);

    ecs_filter_t filter;
    ecs_filter_t *filter_ptr = NULL;

    if(lua_gettop(L) > 1)
    {
        checkfilter(L, w, &filter, 2);
        filter_ptr = &filter;
    }

    ecs_iter_t it = ecs_snapshot_iter(snapshot, filter_ptr);

    ecs_iter_to_lua(&it, L, true);

    return 1;
}

int snapshot_next(lua_State *L)
{
    ecs_iter_t *it = ecs_lua_to_iter(L, 1);

    int b = ecs_snapshot_next(it);

    if(b) ecs_lua_iter_update(L, 1, it);

    lua_pushboolean(L, b);

    return 1;
}