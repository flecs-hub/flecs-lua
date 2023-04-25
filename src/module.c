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

    ecs_component_desc_t desc = {0};
    ecs_entity_t e = ecs_module_init(w, m->name, &desc);

    ecs_set_scope(w, e);

    ctx->error = lua_pcall(L, 0, 0, 0);
}

static void export_handles(lua_State *L, int idx, ecs_world_t *w, ecs_entity_t e)
{
    idx = lua_absindex(L, idx);

    luaL_checktype(L, idx, LUA_TTABLE);

    ecs_iter_t it = ecs_term_iter(w, &(ecs_term_t){ .id = ecs_pair(EcsChildOf, e)});

    while(ecs_term_next(&it))
    {
        int i, type;
        const char *name;

        for(i=0; i < it.count; i++)
        {
            e = it.entities[i];
            name = ecs_get_name(it.world, e);
            if(!name) continue;

            lua_pushinteger(L, e);

            type = lua_getfield(L, idx, name);

            if(type != LUA_TNIL && type != LUA_TFUNCTION)
            {
                if(!lua_rawequal(L, -1, -2)) luaL_error(L, "export table conflict (%s)", name);
            }

            lua_pop(L, 1);

            lua_setfield(L, idx, name);
        }
    }
}

int new_module(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L, w);

    const char *name = luaL_checkstring(L, 1);

    int func_idx = 2;

    if(lua_type(L, 2) == LUA_TTABLE) func_idx = 3;

    luaL_checktype(L, func_idx, LUA_TFUNCTION);

    char *module_name = ecs_module_path_from_c(name);

    ctx->error = 0;
    ecs_lua_module m = { .ctx = ctx, .name = module_name };

    void *orig = ecs_get_context(w);

    ecs_set_context(w, &m);
    ecs_entity_t e = ecs_import(w, import_entry_point, module_name);
    ecs_set_context(w, orig);

    ecs_os_free(module_name);

    if(ctx->error) return lua_error(L);

    if(func_idx == 3) export_handles(L, 2, w, e);

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

    if(!e || !ecs_has_id(w, e, EcsModule)) return luaL_argerror(L, 1, "no such module");

    if(lua_type(L, 2) != LUA_TTABLE) lua_createtable(L, 0, 4);

    export_handles(L, -1, w, e);

    return 1;
}
