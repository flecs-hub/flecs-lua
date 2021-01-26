#include "private.h"

typedef struct ecs_lua_module
{
    ecs_lua_ctx *ctx;
    const char *name;
    int imported;
}ecs_lua_module;

static void import_entry_point(ecs_world_t *w)
{
    ecs_lua_module *m = ecs_get_context(w);
    ecs_lua_ctx *ctx = m->ctx;
    lua_State *L = ctx->L;

    m->imported = 1;

    ecs_new_module(w, 0, m->name, 4, 4);

    ctx->error = lua_pcall(L, 0, 0, 0);
}

int new_module(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L, w);

    const char *name = luaL_checkstring(L, 1);

    luaL_checktype(L, 2, LUA_TFUNCTION);

    ctx->error = 0;
    ecs_lua_module m = { .ctx = ctx, .name = name };

    void *orig = ecs_get_context(w);

    ecs_set_context(w, &m);
    ecs_entity_t e = ecs_import(w, import_entry_point, name, NULL, 0);
    ecs_set_context(w, orig);

    if(ctx->error) return lua_error(L);

    lua_pushinteger(L, e);

    return 1;
}

int import_handles(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    const char *c_name = luaL_checkstring(L, 1);

    const char *name = ecs_module_path_from_c(c_name);
    ecs_entity_t e = ecs_lookup_fullpath(w, name);
    ecs_os_free((char*)name);

    if(!e || !ecs_has_entity(w, e, EcsModule)) return luaL_argerror(L, 1, "no such module");

    ecs_filter_t filter =
    {
        .include = ecs_type(EcsName)
    };

    ecs_iter_t it = ecs_scope_iter_w_filter(w, e, &filter);

    lua_createtable(L, 0, 4);

    int i;
    while(ecs_scope_next(&it))
    {
        for(i=0; i < it.count; i++)
        {
            e = it.entities[i];
            name = ecs_get_name(w, e);
            if(!name) continue;

            lua_pushinteger(L, e);
            lua_setfield(L, -2, name);
        }
    }

    return 1;
}
