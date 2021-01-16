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
