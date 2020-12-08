#include "private.h"

typedef struct ecs_lua_col_t
{
    ecs_entity_t type;
    size_t stride;
    void *ptr;
    bool readback, write;
}ecs_lua_col_t;

typedef struct ecs_lua_each_t
{
    ecs_iter_t *it;
    int32_t i;
    bool from_query, read_prev;
    ecs_lua_col_t cols[];
}ecs_lua_each_t;

ecs_query_t *checkquery(lua_State *L, int arg)
{
    ecs_query_t **query = luaL_checkudata(L, 1, "ecs_query_t");

    if(ecs_query_orphaned(*query)) luaL_argerror(L, 1, "parent query was collected");

    return *query;
}

int query_gc(lua_State *L)
{ecs_os_dbg("QUERY_GC"); fflush(stdout);
    ecs_query_t *query = checkquery(L, 1);

    ecs_query_free(query);

    return 0;
}

int query_new(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    const char *sig = luaL_checkstring(L, 1);
    ecs_query_t *query = ecs_query_new(w, sig);

    ecs_query_t **ptr = lua_newuserdata(L, sizeof(ecs_query_t*));
    *ptr = query;

    luaL_setmetatable(L, "ecs_query_t");

    return 1;
}

int subquery_new(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_query_t *parent = checkquery(L, 1);
    const char *sig = luaL_checkstring(L, 2);

    ecs_query_t *query = ecs_subquery_new(w, parent, sig);

    ecs_query_t **ptr = lua_newuserdata(L, sizeof(ecs_query_t*));
    *ptr = query;

    return 1;
}

int query_iter(lua_State *L)
{ecs_os_dbg("QUERY_iter");
    ecs_query_t *query = checkquery(L, 1);

    ecs_iter_t it = ecs_query_iter(query);

    /* will push with no columns because it->count = 0 */
    ecs_iter_to_lua(&it, L, true);

    return 1;
}

int query_next(lua_State *L)
{
    int b = ecs_lua_query_next(L, 1);

    lua_pushboolean(L, b);

    return 1;
}

int query_changed(lua_State *L)
{
    ecs_query_t *query = checkquery(L, 1);

    int b = ecs_query_changed(query);

    lua_pushboolean(L, b);

    return 1;
}

static void each_reset_columns(ecs_lua_each_t *each)
{
    ecs_iter_t *it = each->it;
    ecs_lua_col_t *col = each->cols;

    each->i = 0;

    if(!it->count) return;

    int i;
    for(i=1; i <= it->column_count; i++, col++)
    {
        col->type = ecs_get_typeid(it->world, ecs_column_entity(it, i));
        col->stride = ecs_column_size(it, i);
        col->ptr = ecs_column_w_size(it, 0, i);

        if(it->query && ecs_is_readonly(it, i)) col->readback = false;
        else col->readback = true;

        col->write = true;
    }
}

static int next_func(lua_State *L)
{
    ecs_lua_each_t *each = lua_touserdata(L, lua_upvalueindex(1));
    ecs_lua_col_t *col = each->cols;
    ecs_iter_t *it = each->it;
    int idx, j, i = each->i;
    bool end = false;
    void *ptr;

    ecs_lua_dbg("each() i: %d", i);

    if(!each->read_prev) goto skip_readback;

    for(j=0; j < it->column_count; j++, col++)
    {
        if(!col->readback) continue;

        ecs_lua_dbg("each() readback: %d", i-1);

        idx = lua_upvalueindex(j+2);
        ptr = ECS_OFFSET(col->ptr, col->stride * (i - 1));

        ecs_lua_to_ptr(it->world, L, idx, col->type, ptr);
    }

    col = each->cols;

skip_readback:

    each->read_prev = true;

    if(i == it->count)
    {
        if(each->from_query)
        {
            if(ecs_lua_query_next(L, 1)) each_reset_columns(each);
            else end = true;
        }
        else end = true;
    }

    if(end) return 0;

    for(j=0; j < it->column_count; j++, col++)
    {// optimization: shared components should be read back at the end
        if(!col->write) continue;

        idx = lua_upvalueindex(j+2);
        ptr = ECS_OFFSET(col->ptr, col->stride * i);

        lua_pushvalue(L, idx);
        ecs_lua_update_type(it->world, L, idx, col->type, ptr);
    }

    lua_pushinteger(L, it->entities[i]);

    each->i++;

    return it->column_count + 1;
}

int each_func(lua_State *L)
{ecs_lua_dbg("ecs.each()");
    ecs_query_t *q = NULL;
    ecs_iter_t *it;
    int iter_idx = 1;

    if(lua_type(L, 1) == LUA_TUSERDATA)
    {
        q = checkquery(L, 1);
        ecs_iter_t iter = ecs_query_iter(q);
        ecs_query_next(&iter);
        it = ecs_iter_to_lua(&iter, L, true);
        iter_idx = lua_gettop(L);
    }
    else it = ecs_lua__checkiter(L, 1);

    size_t size = sizeof(ecs_lua_each_t) + it->column_count * sizeof(ecs_lua_col_t);
    ecs_lua_each_t *each = lua_newuserdata(L, size);

    each->it = it;
    each->from_query = q ? true : false;
    each->read_prev = false;

    each_reset_columns(each);

    int i;
    for(i=0; i < it->column_count; i++)
    {
        lua_newtable(L);
    }

    lua_pushcclosure(L, next_func, it->column_count + 1);

    /* it */
    lua_pushvalue(L, iter_idx);

    lua_pushinteger(L, 1);

    return 3;
}
