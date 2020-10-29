#include "private.h"

static const int ecs_lua__key;

#define ECS_LUA_CONTEXT (&ecs_lua__key)

#define ECS_LUA__KEEPOPEN 1

ecs_lua_ctx *ecs_lua_get_context(lua_State *L)
{
    lua_rawgetp(L, LUA_REGISTRYINDEX, ECS_LUA_CONTEXT);
    ecs_lua_ctx *p = lua_touserdata(L, -1);
    lua_pop(L, 1);
    return p;
}

ecs_world_t *ecs_lua_get_world(lua_State *L)
{
    ecs_lua_ctx *p = ecs_lua_get_context(L);
    return p->world;
}

void ecs_lua_progress(lua_State *L)
{
    ecs_lua__prolog(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);

    ecs_assert(ctx->progress_ref != LUA_NOREF, ECS_INVALID_PARAMETER, "progress callback is not set");

    if(ctx->progress_ref == LUA_NOREF) return;

    lua_rawgeti(L, LUA_REGISTRYINDEX, ctx->progress_ref);

    int type = lua_type(L, -1);
    ecs_assert(type == LUA_TFUNCTION, ECS_INTERNAL_ERROR, NULL);

    if(type != LUA_TFUNCTION) return;

    lua_pcall(L, 0, 0, 0);

    ecs_lua__epilog(L);
}

static int func(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);
    return 1;
}

/* Entity */
int new_entity(lua_State *L);
int delete_entity(lua_State *L);
int new_tag(lua_State *L);
int entity_name(lua_State *L);
int entity_symbol(lua_State *L);
int entity_fullpath(lua_State *L);
int lookup_entity(lua_State *L);
int lookup_fullpath(lua_State *L);
int entity_has(lua_State *L);
int has_role(lua_State *L);
int is_alive(lua_State *L);
int exists(lua_State *L);
int add_type(lua_State *L);
int remove_type(lua_State *L);
int clear_entity(lua_State *L);
int get_type(lua_State *L);

int new_array(lua_State *L);
int new_struct(lua_State *L);
int new_alias(lua_State *L);

int get_func(lua_State *L);
int get_mut(lua_State *L);

int mutable_modified(lua_State *L);
int set_func(lua_State *L);

int singleton_get(lua_State *L);
int singleton_get_mut(lua_State *L);
int mutable_modified(lua_State *L);
int singleton_set(lua_State *L);

int new_prefab(lua_State *L);

/* Bulk */
int bulk_new(lua_State *L);
int bulk_delete(lua_State *L);

/* Iterator */
int column(lua_State *L);
int columns(lua_State *L);
int is_owned(lua_State *L);

/* Query */
int query_gc(lua_State *L);
int query_new(lua_State *L);
int query_iter(lua_State *L);
int query_next(lua_State *L);
int query_changed(lua_State *L);

/* System */
int new_system(lua_State *L);

/* Module */
int new_module(lua_State *L);

/* Log */
int print_log(lua_State *L);
int print_err(lua_State *L);
int print_dbg(lua_State *L);
int print_warn(lua_State *L);
int tracing_enable(lua_State *L);

/* Misc */
int assert_func(lua_State *L);
int sizeof_component(lua_State *L);

/* Time */
int get_time(lua_State *L);
int time_measure(lua_State *L);
int time__tostring(lua_State *L);

/* Pipeline */
int new_pipeline(lua_State *L);
int set_pipeline(lua_State *L);
int get_pipeline(lua_State *L);
int progress(lua_State *L);
int progress_cb(lua_State *L);
int set_target_fps(lua_State *L);
int set_time_scale(lua_State *L);
int reset_clock(lua_State *L);
int lquit(lua_State *L);
int deactivate_systems(lua_State *L);
int set_threads(lua_State *L);
int get_threads(lua_State *L);
int get_thread_index(lua_State *L);

/* World */
int world_info(lua_State *L);
int dim(lua_State *L);
int dim_type(lua_State *L);

/* EmmyLua */
int emmy_class(lua_State *L);

static const luaL_Reg ecs_lib[] =
{
    { "new", new_entity },
    { "delete", delete_entity },
    { "tag", new_tag },
    { "name", entity_name },
    { "symbol", entity_symbol },
    { "fullpath", entity_fullpath },
    { "lookup", lookup_entity },
    { "lookup_fullpath", lookup_fullpath },
    { "has", entity_has },
    { "has_role", has_role },
    { "is_alive", is_alive },
    { "exists", exists },
    { "add", add_type },
    { "remove", remove_type },
    { "clear", clear_entity },
    { "get_type", get_type },

    { "array", new_array },
    { "struct", new_struct },
    { "alias", new_alias },

    { "get", get_func },
    { "get_mut", get_mut },
    { "modified", mutable_modified },
    { "set", set_func },

    { "singleton_get", singleton_get },
    { "singleton_get_mut", singleton_get_mut },
    { "singleton_modified", mutable_modified },
    { "singleton_set", singleton_set },

    { "prefab", new_prefab },

    { "bulk_new", bulk_new },
    { "bulk_delete", bulk_delete },

    { "column", column },
    { "columns", columns },
    { "is_owned", is_owned },

    { "query", query_new },
    { "query_iter", query_iter },
    { "query_next", query_next },
    { "query_changed", query_changed },

    { "system", new_system },

    { "module", new_module },

    { "log", print_log },
    { "err", print_err },
    { "dbg", print_dbg },
    { "warn", print_warn },
    { "tracing_enable", tracing_enable },

    { "assert", assert_func },
    { "sizeof", sizeof_component },

    { "get_time", get_time },
    { "time_measure", time_measure },

    { "pipeline", new_pipeline },
    { "set_pipeline", set_pipeline },
    { "get_pipeline", get_pipeline },
    { "progress", progress },
    { "progress_cb", progress_cb },
    { "set_target_fps", set_target_fps },
    { "set_time_scale", set_time_scale },
    { "reset_clock", reset_clock },
    { "quit", lquit },
    { "deactivate_systems", deactivate_systems },
    { "set_threads", set_threads },
    { "get_threads", get_threads },
    { "get_thread_index", get_thread_index },

    { "world_info", world_info },
    { "dim", dim },
    { "dim_type", dim_type },

    { "emmy_class", emmy_class },

#define XX(const) {#const, NULL },
    ECS_LUA_ENUMS(XX)
    ECS_LUA_MACROS(XX)
#undef XX
    { NULL, NULL }
};

int luaopen_ecs(lua_State *L)
{
    luaL_newlib(L, ecs_lib);

    luaL_newmetatable(L, "ecs_type_t");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ecs_readonly");
    lua_pushcfunction(L, ecs_lua__readonly);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, ecs_lua__readonly);
    lua_setfield(L, -2, "__usedindex");
    lua_pushboolean(L, false);
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ecs_query_t");
    lua_pushcfunction(L, query_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ecs_time_t");
    lua_pushcfunction(L, time__tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pop(L, 1);

    lua_createtable(L, 128, 0);
    lua_rawsetp(L, LUA_REGISTRYINDEX, ECS_LUA_CURSORS);

    lua_createtable(L, 128, 0);
    lua_rawsetp(L, LUA_REGISTRYINDEX, ECS_LUA_TYPES);

#define XX(const) lua_pushinteger(L, Ecs##const); lua_setfield(L, -2, #const);
    ECS_LUA_ENUMS(XX)
#undef XX
#define XX(const) lua_pushinteger(L, ECS_##const); lua_setfield(L, -2, #const);
    ECS_LUA_MACROS(XX)
#undef XX
    return 1;
}

static ecs_lua_ctx *ctx_init(ecs_lua_ctx ctx)
{
    lua_State *L = ctx.L;
    ecs_world_t *world = ctx.world;

    ecs_lua_ctx *lctx = lua_newuserdata(L, sizeof(ecs_lua_ctx));
    lua_rawsetp(L, LUA_REGISTRYINDEX, ECS_LUA_CONTEXT);

    memcpy(lctx, &ctx, sizeof(ecs_lua_ctx));

    lctx->error = 0;
    lctx->progress_ref = LUA_NOREF;

    ecs_entity_t MetaTypeSerializer = ecs_lookup_fullpath(world, "flecs.meta.MetaTypeSerializer");
    ecs_assert(MetaTypeSerializer != 0, ECS_INTERNAL_ERROR, NULL);

    lctx->serializer_id = MetaTypeSerializer;

    lctx->metatype_id = ecs_lookup_fullpath(world, "flecs.meta.MetaTypeSerializer");
    ecs_assert(lctx->metatype_id != 0, ECS_INTERNAL_ERROR, NULL);

    lua_pushinteger(L, MetaTypeSerializer);
    lua_rawsetp(L, LUA_REGISTRYINDEX, ECS_LUA_SERIALIZER);

    luaL_requiref(L, "ecs", luaopen_ecs, 1);
    lua_pop(L, 1);

    return lctx;
}

static void ecs_lua_exit(lua_State *L)
{
    ecs_lua_ctx *ctx = ecs_lua_get_context(L);

    if( !(ctx->internal & ECS_LUA__KEEPOPEN) ) lua_close(L);
}

int ecs_lua_set_state(ecs_world_t *w, lua_State *L)
{
    ecs_entity_t ecs_entity(EcsLuaHost) = ecs_lookup_fullpath(w, "flecs.lua.LuaHost");
    ecs_assert(ecs_entity(EcsLuaHost) != 0, ECS_INTERNAL_ERROR, NULL);

    EcsLuaHost *host = ecs_singleton_get_mut(w, EcsLuaHost);

    ecs_lua_exit(host->L);

    ecs_lua_ctx param = { .L = L, .world = w, .internal = ECS_LUA__KEEPOPEN };

    host->L = L;
    host->ctx = ctx_init(param);

    ecs_singleton_modified(w, EcsLuaHost);

    return 0;
}

static void *Allocf(void *ud, void *ptr, size_t osize, size_t nsize)
{
    if(!nsize)
    {
        ecs_os_free(ptr);
        return NULL;
    }

    return ecs_os_realloc(ptr, nsize);
}


/* Should only be called on ecs_fini() */
ECS_DTOR(EcsLuaHost, ptr,
{
    lua_close(ptr->L);
    ptr->L = NULL;
});

void FlecsLuaImport(ecs_world_t *w)
{
    ECS_MODULE(w, FlecsLua);

    ECS_IMPORT(w, FlecsMeta);

    ecs_set_name_prefix(w, "Ecs");

    ECS_COMPONENT(w, EcsLuaHost);

    ECS_META(w, EcsLuaWorldInfo);

    ECS_EXPORT_COMPONENT(EcsLuaHost);

    ecs_assert(sizeof(EcsLuaWorldInfo) == sizeof(ecs_world_info_t), ECS_INTERNAL_ERROR, NULL);

    lua_State *L = lua_newstate(Allocf, NULL);

    ecs_lua_ctx param = { .L = L, .world = w};
    ecs_lua_ctx *ctx = ctx_init(param);

    ecs_singleton_set(w, EcsLuaHost, { .L = L, .ctx = ctx });

    ecs_set_component_actions(w, EcsLuaHost,
    {
        .dtor = ecs_dtor(EcsLuaHost)
    });
}
