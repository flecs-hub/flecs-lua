#include "private.h"


ecs_iter_t *ecs_lua__checkiter(lua_State *L, int arg)
{
    if(luaL_getmetafield(L, arg, "__ecs_iter") == LUA_TNIL)
        luaL_argerror(L, arg, "table is not an iterator");

    ecs_iter_t *it = lua_touserdata(L, -1);
    lua_pop(L, 1);

    return it;
}

static inline void copy_term_id(ecs_term_id_t *dst, EcsLuaTermID *src)
{
    dst->id = src->id;
    dst->trav = src->trav;
    dst->flags = src->flags;
}

static inline void copy_term(ecs_term_t *dst, EcsLuaTerm *src)
{
    dst->id = src->id;

    dst->inout = src->inout;
    copy_term_id(&dst->src, &src->src);
    copy_term_id(&dst->first, &src->first);
    copy_term_id(&dst->second, &src->second);
    dst->oper = src->oper;
}

ecs_term_t checkterm(lua_State *L, const ecs_world_t *world, int arg)
{
    ecs_term_t term = {0};

    int type = lua_type(L, arg);

    if(type == LUA_TTABLE)
    {
        EcsLuaTerm lua_term = {0};

        ecs_lua_to_ptr(world, L, arg, ecs_id(EcsLuaTerm), &lua_term);

        copy_term(&term, &lua_term);
    }
    else
    {
        term.id = luaL_checkinteger(L, arg);
    }

    if(ecs_term_finalize(world, &term)) luaL_argerror(L, arg, "invalid term");

    if(term.id == 0) luaL_argerror(L, arg, "empty term");

    return term;
}

static ecs_iter_t *get_iter_columns(lua_State *L, int arg)
{
    ecs_iter_t *it = ecs_lua__checkiter(L, arg);

    luaL_getsubtable(L, arg, "columns");

    return it;
}

int iter_term(lua_State *L)
{
    ecs_iter_t *it = get_iter_columns(L, 1);

    lua_Integer i = luaL_checkinteger(L, 2);

    if(i < 1 || i > it->field_count) return luaL_argerror(L, 2, "invalid term index");

    lua_geti(L, -1, i);

    return 1;
}

int iter_terms(lua_State *L)
{
    ecs_iter_t *it = get_iter_columns(L, 1);

    int i;
    for(i=1; i <= it->field_count; i++)
    {
        lua_geti(L, 2, i);
    }

    lua_getfield(L, 1, "entities");

    return it->field_count + 1;
}

int is_owned(lua_State *L)
{
    ecs_iter_t *it = ecs_lua__checkiter(L, 1);
    lua_Integer col = luaL_checkinteger(L, 2);

    if(col < 1 || col > it->field_count) luaL_argerror(L, 2, "invalid field index");

    int b = ecs_field_is_self(it, col);

    lua_pushboolean(L, b);

    return 1;
}

int term_id(lua_State *L)
{
    ecs_iter_t *it = ecs_lua__checkiter(L, 1);
    lua_Integer col = luaL_checkinteger(L, 2);

    if(col < 1 || col > it->field_count) luaL_argerror(L, 2, "invalid field index");

    ecs_entity_t e = ecs_field_id(it, col);

    lua_pushinteger(L, e);

    return 1;
}

int filter_iter(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_filter_t *filter = checkfilter(L, w, 1);

    ecs_iter_t it = ecs_filter_iter(w, filter);

    ecs_iter_to_lua(&it, L, true);

    return 1;
}

int filter_next(lua_State *L)
{
    ecs_iter_t *it = ecs_lua_to_iter(L, 1);

    int b = ecs_filter_next(it);

    if(b) ecs_lua_iter_update(L, 1, it);

    lua_pushboolean(L, b);

    return 1;
}

int term_iter(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_term_t term = checkterm(L, w, 1);

    ecs_iter_t it = ecs_term_iter(w, &term);

    ecs_iter_to_lua(&it, L, true);

    return 1;
}

int term_next(lua_State *L)
{
    ecs_iter_t *it = ecs_lua_to_iter(L, 1);

    int b = ecs_term_next(it);

    if(b) ecs_lua_iter_update(L, 1, it);

    lua_pushboolean(L, b);

    return 1;
}

int iter_next(lua_State *L)
{
    int b = ecs_lua_iter_next(L, 1);

    lua_pushboolean(L, b);

    return 1;
}