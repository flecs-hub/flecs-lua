#include "private.h"


ecs_type_t checktype(lua_State *L, int arg)
{
    ecs_type_t *type = luaL_checkudata(L, arg, "ecs_type_t");

    return *type;
}

int check_filter_desc(lua_State *L, const ecs_world_t *world, ecs_filter_desc_t *desc, int arg)
{
    luaL_checktype(L, arg, LUA_TTABLE);

    ecs_term_t *terms = desc->terms;

    int fields = 0;
    int terms_type = lua_getfield(L, arg, "terms");
    int expr_type = lua_getfield(L, arg, "expr");

    if(terms_type != LUA_TNIL)
    {
        int i, len = 0;

        if(terms_type == LUA_TNUMBER) /* terms: integer */
        {
            terms[0] = checkterm(L, world, -2);
        }
        else if(terms_type == LUA_TTABLE) /* terms: ecs_term_t|ecs_term_t[] */
        {
            int type = lua_rawgeti(L, -2, 1); /* type(terms[1]) */
            lua_pop(L, 1);

            if(type != LUA_TNIL) /* terms: ecs_term_t[] */
            {
                len = luaL_len(L, -2);

                if(len > FLECS_TERM_DESC_MAX) return luaL_argerror(L, arg, "too many terms");

                for(i=0; i < len; i++)
                {
                    lua_rawgeti(L, -2, i + 1);
                    terms[i] = checkterm(L, world, -1);
                    lua_pop(L, 1);
                }
            }
            else /* terms: ecs_term_t */
            {
                terms[0] = checkterm(L, world, -2);
            }
        }
        else return luaL_argerror(L, arg, "invalid term type");

        fields++;
    }

    if(expr_type != LUA_TNIL)
    {
        if(expr_type != LUA_TSTRING) return luaL_argerror(L, arg, "expected string (expr)");

        desc->expr = (char*)luaL_checkstring(L, -1);

        fields++;
    }

    lua_pop(L, 2);

    if(!fields) return luaL_argerror(L, arg, "empty filter");

    return 0;
}

ecs_filter_t *checkfilter(lua_State *L, const ecs_world_t *world, int arg)
{
    ecs_filter_desc_t filter_desc = {0};

    check_filter_desc(L, world, &filter_desc, arg);

    ecs_filter_t *filter = ecs_filter_init(world, &filter_desc);

    if(!filter) luaL_argerror(L, arg, "invalid filter");

    return filter;
}

int assert_func(lua_State *L)
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

int sizeof_component(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    const EcsComponent *ptr = ecs_get(w, e, EcsComponent);

    if(!ptr) luaL_argerror(L, 1, "not a component");

    lua_pushinteger(L, ptr->size);
    lua_pushinteger(L, ptr->alignment);

    return 2;
}

int is_primitive(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t e = luaL_checkinteger(L, 1);

    luaL_argcheck(L, e != 0, 1, "expected non-zero entity");

    const EcsPrimitive *p = ecs_get(w, e, EcsPrimitive);

    if(p == NULL) return 0;

    lua_pushinteger(L, p->kind);

    return 1;
}

int createtable(lua_State *L)
{
    int narr = luaL_optinteger(L, 1, 0);
    int nrec = luaL_optinteger(L, 2, 0);

    lua_createtable(L, narr, nrec);

    return 1;
}

int ecs_lua__readonly(lua_State *L)
{
    return luaL_error(L, "Attempt to modify read-only table");
}

int zero_init_component(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t component = luaL_checkinteger(L, 1);

    ecs_type_hooks_t hooks =
    {
        .ctor = ecs_default_ctor
    };

    ecs_set_hooks_id(w, component, &hooks);

    return 0;
}

int get_world_ptr(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    lua_pushlightuserdata(L, w);

    return 1;
}

void ecs_lua__assert(lua_State *L, bool condition, const char *param, const char *condition_str)
{
    if(!condition) luaL_error(L, "assert(%s) failed", condition_str);
}