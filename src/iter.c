#include "private.h"


ecs_iter_t *ecs_lua__checkiter(lua_State *L, int arg)
{
    if(luaL_getmetafield(L, arg, "__ecs_iter") == LUA_TNIL)
        luaL_argerror(L, arg, "table is not an iterator");

    ecs_iter_t *it = lua_touserdata(L, -1);
    lua_pop(L, 1);

    return it;
}

static inline void copy_term_set(ecs_term_set_t *dst, EcsLuaTermSet *src)
{
    dst->relation = src->relation;
    dst->mask = src->mask;
    dst->min_depth = src->min_depth;
    dst->max_depth = src->max_depth;
}

static inline void copy_term_id(ecs_term_id_t *dst, EcsLuaTermID *src)
{
    dst->entity = src->entity;
    dst->var = src->var;
    copy_term_set(&dst->set, &src->set);
}

static inline void copy_term(ecs_term_t *dst, EcsLuaTerm *src)
{
    dst->id = src->id;

    dst->inout = src->inout;
    copy_term_id(&dst->pred, &src->pred);
    copy_term_id(&dst->args[0], &src->subj);
    copy_term_id(&dst->args[1], &src->obj);
    dst->oper = src->oper;
    dst->role = src->role;
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

    if(ecs_term_finalize(world, NULL, NULL, &term)) luaL_argerror(L, arg, "invalid term");

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

    if(i < 1 || i > it->column_count) return luaL_argerror(L, 2, "invalid term index");

    lua_geti(L, -1, i);

    return 1;
}

int iter_terms(lua_State *L)
{
    ecs_iter_t *it = get_iter_columns(L, 1);

    int i;
    for(i=1; i <= it->column_count; i++)
    {
        lua_geti(L, 2, i);
    }

    lua_getfield(L, 1, "entities");

    return it->column_count + 1;
}

int is_owned(lua_State *L)
{
    ecs_iter_t *it = ecs_lua__checkiter(L, 1);
    lua_Integer col = luaL_checkinteger(L, 2);

    if(col < 1 || col > it->column_count) luaL_argerror(L, 2, "invalid term index");

    int b = ecs_term_is_owned(it, col);

    lua_pushboolean(L, b);

    return 1;
}

int term_id(lua_State *L)
{
    ecs_iter_t *it = ecs_lua__checkiter(L, 1);
    lua_Integer col = luaL_checkinteger(L, 2);

    if(col < 1 || col > it->column_count) luaL_argerror(L, 2, "invalid term index");

    ecs_entity_t e = ecs_term_id(it, col);

    lua_pushinteger(L, e);

    return 1;
}

int filter_iter(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_filter_t filter;
    checkfilter(L, w, &filter, 1);

    ecs_iter_t it = ecs_filter_iter(w, &filter);

    ecs_filter_fini(&filter);

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
