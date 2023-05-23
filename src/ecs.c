#define FLECS_LUA_IMPL
#include "private.h"

ECS_COMPONENT_DECLARE(EcsLuaHost);

static const int ecs_lua__ctx;
static const int ecs_lua__world;

#define ECS_LUA_DEFAULT_CTX (&ecs_lua__ctx)
#define ECS_LUA_DEFAULT_WORLD (&ecs_lua__world)

#define ECS_LUA__KEEPOPEN 1
#define ECS_LUA__DYNAMIC 2 /* Loaded as Lua module */

static ecs_lua_ctx *ctx_init(ecs_lua_ctx ctx);

ecs_lua_ctx *ecs_lua_get_context(lua_State *L, const ecs_world_t *world)
{
    int type;
    ecs_lua_ctx *ctx;

    if(world)
    {
        type = lua_rawgetp(L, LUA_REGISTRYINDEX, world);
        ecs_assert(type == LUA_TTABLE || type == LUA_TNIL, ECS_INTERNAL_ERROR, NULL);

        if(type == LUA_TNIL) return NULL;

        type = lua_rawgeti(L, -1, ECS_LUA_CONTEXT);
        ecs_assert(type == LUA_TUSERDATA, ECS_INTERNAL_ERROR, NULL);

        ctx = lua_touserdata(L, -1);
        lua_pop(L, 2);

        return ctx;
    }

    lua_rawgetp(L, LUA_REGISTRYINDEX, ECS_LUA_DEFAULT_CTX);
    ctx = lua_touserdata(L, -1);
    lua_pop(L, 1);

    return ctx;
}

ecs_world_t *ecs_lua_get_world(lua_State *L)
{
    ecs_lua_ctx *ctx = ecs_lua_get_context(L, NULL);
    return ctx ? ctx->world : NULL;
}

bool ecs_lua_progress(lua_State *L, lua_Number delta_time)
{
    ecs_lua__prolog(L);
    ecs_lua_ctx *ctx = ecs_lua_get_context(L, NULL);

    ecs_assert(ctx->progress_ref != LUA_NOREF, ECS_INVALID_PARAMETER, "progress callback is not set");

    if(ctx->progress_ref == LUA_NOREF) return false;

    lua_rawgeti(L, LUA_REGISTRYINDEX, ctx->progress_ref);

    int type = lua_type(L, -1);
    ecs_assert(type == LUA_TFUNCTION, ECS_INTERNAL_ERROR, NULL);
    if(type != LUA_TFUNCTION) return false;

    lua_pushnumber(L, delta_time);

    int ret = lua_pcall(L, 1, 1, 0);

    if(ret)
    {
        const char *err = lua_tostring(L, lua_gettop(L));
        ecs_os_err("progress() cb error (%d): %s", ret, err);
        lua_pop(L, 1);
        return false;
    }

    ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);

    int b = lua_toboolean(L, lua_gettop(L));
    lua_pop(L, 1);

    ecs_lua__epilog(L);

    return b;
}

void register_collectible(lua_State *L, ecs_world_t *w, int idx)
{
    ecs_lua__prolog(L);
    idx = lua_absindex(L, idx);
    int type = lua_rawgetp(L, LUA_REGISTRYINDEX, w);
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    /* registry[world].collect */
    type = lua_rawgeti(L, -1, ECS_LUA_COLLECT);
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    lua_type(L, idx);
    lua_pushvalue(L, idx);
    lua_pushboolean(L, 1); /* dummy value */
    lua_settable(L, -3); /* collect[obj] = true */

    lua_pop(L, 2);
    ecs_lua__epilog(L);
}

int ecs_lua_ref(lua_State *L, ecs_world_t *world)
{
    int type = lua_rawgetp(L, LUA_REGISTRYINDEX, ecs_get_world(world));
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    type = lua_rawgeti(L, -1, ECS_LUA_REGISTRY);
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    lua_pushvalue(L, -3);
    int ref = luaL_ref(L, -2);

    lua_pop(L, 3);

    return ref;
}

int ecs_lua_rawgeti(lua_State *L, ecs_world_t *world, int ref)
{
    int type = lua_rawgetp(L, LUA_REGISTRYINDEX, ecs_get_world(world));
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    type = lua_rawgeti(L, -1, ECS_LUA_REGISTRY);
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    type = lua_rawgeti(L, -1, ref);

    lua_replace(L, -3);

    lua_pop(L, 1);

    return type;
}

void ecs_lua_unref(lua_State *L, ecs_world_t *world, int ref)
{
    ecs_lua__prolog(L);

    int type = lua_rawgetp(L, LUA_REGISTRYINDEX, ecs_get_world(world));
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    type = lua_rawgeti(L, -1, ECS_LUA_REGISTRY);
    ecs_assert(type == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    luaL_unref(L, -1, ref);

    lua_pop(L, 2);

    ecs_lua__epilog(L);
}


/* Entity */
int new_entity(lua_State *L);
int new_id(lua_State *L);
int delete_entity(lua_State *L);
int new_tag(lua_State *L);
int entity_name(lua_State *L);
int set_name(lua_State *L);
int entity_symbol(lua_State *L);
int entity_fullpath(lua_State *L);
int lookup_entity(lua_State *L);
int lookup_child(lua_State *L);
int lookup_path(lua_State *L);
int lookup_fullpath(lua_State *L);
int lookup_symbol(lua_State *L);
int use_alias(lua_State *L);
int entity_has(lua_State *L);
int entity_owns(lua_State *L);
int is_alive(lua_State *L);
int is_valid(lua_State *L);
int get_alive(lua_State *L);
int ensure(lua_State *L);
int exists(lua_State *L);
int entity_add(lua_State *L);
int entity_remove(lua_State *L);
int clear_entity(lua_State *L);
int enable_entity(lua_State *L);
int disable_entity(lua_State *L);
int entity_count(lua_State *L);
int delete_children(lua_State *L);
int get_type(lua_State *L);
int get_typeid(lua_State *L);
int get_parent(lua_State *L);

int enable_component(lua_State *L);
int disable_component(lua_State *L);
int is_component_enabled(lua_State *L);

int add_pair(lua_State *L);
int remove_pair(lua_State *L);
int has_pair(lua_State *L);
int set_pair(lua_State *L);
int set_pair_object(lua_State *L);
int get_pair(lua_State *L);
int get_mut_pair(lua_State *L);
int get_pair_object(lua_State *L);
int get_mut_pair_object(lua_State *L);
int make_pair(lua_State *L);
int is_pair(lua_State *L);
int pair_object(lua_State *L);
int add_instanceof(lua_State *L);
int remove_instanceof(lua_State *L);
int add_childof(lua_State *L);
int remove_childof(lua_State *L);
int entity_override(lua_State *L);

int new_enum(lua_State *L);
int new_bitmask(lua_State *L);
int new_array(lua_State *L);
int new_struct(lua_State *L);
int new_alias(lua_State *L);

int get_func(lua_State *L);
int get_mut(lua_State *L);
int patch_func(lua_State *L);
int set_func(lua_State *L);

int new_ref(lua_State *L);
int get_ref(lua_State *L);

int singleton_get(lua_State *L);
int singleton_patch(lua_State *L);
int singleton_set(lua_State *L);

int new_prefab(lua_State *L);

/* Hierarchy */
int get_child_count(lua_State *L);
int set_scope(lua_State *L);
int get_scope(lua_State *L);
int set_name_prefix(lua_State *L);

/* Bulk */
int bulk_new(lua_State *L);

/* Iterator */
int iter_term(lua_State *L);
int iter_terms(lua_State *L);
int is_owned(lua_State *L);
int term_id(lua_State *L);
int filter_iter(lua_State *L);
int filter_next(lua_State *L);
int term_iter(lua_State *L);
int term_next(lua_State *L);
int iter_next(lua_State *L);

/* Query */
int query_gc(lua_State *L);
int query_new(lua_State *L);
int subquery_new(lua_State *L);
int query_iter(lua_State *L);
int query_next(lua_State *L);
int query_changed(lua_State *L);
int each_func(lua_State *L);

/* Snapshot */
int snapshot_take(lua_State *L);
int snapshot_restore(lua_State *L);
int snapshot_iter(lua_State *L);
int snapshot_next(lua_State *L);
int snapshot_gc(lua_State *L);

/* System */
int new_system(lua_State *L);
int new_trigger(lua_State *L);
int new_observer(lua_State *L);
int run_system(lua_State *L);
int set_system_context(lua_State *L);

/* Module */
int new_module(lua_State *L);
int import_handles(lua_State *L);

/* Log */
int print_log(lua_State *L);
int print_err(lua_State *L);
int print_dbg(lua_State *L);
int print_warn(lua_State *L);
int log_set_level(lua_State *L);
int log_enable_colors(lua_State *L);

/* Misc */
int assert_func(lua_State *L);
int sizeof_component(lua_State *L);
int is_primitive(lua_State *L);
int createtable(lua_State *L);
int zero_init_component(lua_State *L);
int get_world_ptr(lua_State *L);
int meta_constants(lua_State *L);

/* Time */
int get_time(lua_State *L);
int time_measure(lua_State *L);
int time__tostring(lua_State *L);

/* Timer */
int set_timeout(lua_State *L);
int get_timeout(lua_State *L);
int set_interval(lua_State *L);
int get_interval(lua_State *L);
int start_timer(lua_State *L);
int stop_timer(lua_State *L);
int set_rate_filter(lua_State *L);
int set_tick_source(lua_State *L);

/* Pipeline */
int new_pipeline(lua_State *L);
int set_pipeline(lua_State *L);
int get_pipeline(lua_State *L);
int progress(lua_State *L);
int progress_cb(lua_State *L);
int measure_frame_time(lua_State *L);
int measure_system_time(lua_State *L);
int set_target_fps(lua_State *L);
int set_time_scale(lua_State *L);
int reset_clock(lua_State *L);
int lquit(lua_State *L);
int set_threads(lua_State *L);
int get_threads(lua_State *L);
int get_thread_index(lua_State *L); //compat

/* World */
int world_new(lua_State *L);
int world_fini(lua_State *L);
int world_gc(lua_State *L);
int world_info(lua_State *L);
int world_stats(lua_State *L);
int dim(lua_State *L);

/* EmmyLua */
int emmy_class(lua_State *L);

static const luaL_Reg ecs_lib[] =
{
    { "new", new_entity },
    { "new_id", new_id },
    { "delete", delete_entity },
    { "tag", new_tag },
    { "name", entity_name },
    { "set_name", set_name },
    { "symbol", entity_symbol },
    { "fullpath", entity_fullpath },
    { "lookup", lookup_entity },
    { "lookup_child", lookup_child },
    { "lookup_path", lookup_path },
    { "lookup_fullpath", lookup_fullpath },
    { "lookup_symbol", lookup_symbol },
    { "use", use_alias },
    { "has", entity_has },
    { "owns", entity_owns },
    { "is_alive", is_alive },
    { "is_valid", is_valid },
    { "get_alive", get_alive },
    { "ensure", ensure },
    { "exists", exists },
    { "add", entity_add },
    { "remove", entity_remove },
    { "clear", clear_entity },
    { "enable", enable_entity },
    { "disable", disable_entity },
    { "count", entity_count },
    { "delete_children", delete_children },
    { "get_parent", get_parent },
    { "get_type", get_type },
    { "get_typeid", get_typeid },

    { "enable_component", enable_component },
    { "disable_component", disable_component },
    { "is_component_enabled", is_component_enabled },

    { "add_pair", add_pair },
    { "remove_pair", remove_pair },
    { "has_pair", has_pair },
    { "set_pair", set_pair },
    { "set_pair_object", set_pair_object },
    { "get_pair", get_pair },
    { "get_mut_pair", get_mut_pair },
    { "get_pair_object", get_pair_object },
    { "get_mut_pair_object", get_mut_pair_object },
    { "pair", make_pair },
    { "is_pair", is_pair },
    { "pair_object", pair_object },
    { "add_instanceof", add_instanceof },
    { "remove_instanceof", remove_instanceof },
    { "add_childof", add_childof },
    { "remove_childof", remove_childof },
    { "override", entity_override },
    { "add_owned", entity_override }, // compat

    { "enum", new_enum },
    { "bitmask", new_bitmask },
    { "array", new_array },
    { "struct", new_struct },
    { "alias", new_alias },

    { "get", get_func },
    { "get_mut", get_mut },
    { "patch", patch_func },
    { "set", set_func },
    { "ref", new_ref },
    { "get_ref", get_ref },

    { "singleton_get", singleton_get },
    { "singleton_patch", singleton_patch },
    { "singleton_set", singleton_set },

    { "prefab", new_prefab },

    { "get_child_count" , get_child_count },
    { "set_scope", set_scope },
    { "get_scope", get_scope },
    { "set_name_prefix", set_name_prefix },

    { "bulk_new", bulk_new },

    { "term", iter_term },
    { "terms", iter_terms },
    { "column", iter_term }, // compat
    { "columns", iter_terms }, // compat
    { "is_owned", is_owned },
    { "column_entity", term_id }, // compat
    { "term_id", term_id },
    { "filter_iter", filter_iter },
    { "filter_next", filter_next },
    { "term_iter", term_iter },
    { "term_next", term_next },
    { "iter_next", iter_next },

    { "query", query_new },
    { "subquery", subquery_new },
    { "query_iter", query_iter },
    { "query_next", query_next },
    { "query_changed", query_changed },
    { "each", each_func },

    { "system", new_system },
    { "trigger", new_trigger },
    { "observer", new_observer },
    { "run", run_system },
    { "set_system_context", set_system_context },

    { "snapshot", snapshot_take },
    { "snapshot_restore", snapshot_restore },
    { "snapshot_iter", snapshot_iter },
    { "snapshot_next", snapshot_next },

    { "module", new_module },
    { "import", import_handles },

    { "log", print_log },
    { "err", print_err },
    { "dbg", print_dbg },
    { "warn", print_warn },
    { "log_set_level", log_set_level },
    { "tracing_enable", log_set_level }, //compat
    { "log_enable_colors", log_enable_colors },
    { "tracing_color_enable", log_enable_colors }, //compat

    { "assert", assert_func },
    { "sizeof", sizeof_component },
    { "is_primitive", is_primitive },
    { "createtable", createtable },
    { "zero_init", zero_init_component },
    { "world_ptr", get_world_ptr },
    { "meta_constants", meta_constants },

    { "get_time", get_time },
    { "time_measure", time_measure },

    { "set_timeout", set_timeout },
    { "get_timeout", get_timeout },
    { "set_interval", set_interval },
    { "get_interval", get_interval },
    { "start_timer", start_timer },
    { "stop_timer", stop_timer },
    { "set_rate_filter", set_rate_filter },
    { "set_tick_source", set_tick_source },

    { "pipeline", new_pipeline },
    { "set_pipeline", set_pipeline },
    { "get_pipeline", get_pipeline },
    { "progress", progress },
    { "progress_cb", progress_cb },
    { "measure_frame_time", measure_frame_time },
    { "measure_system_time", measure_system_time },
    { "set_target_fps", set_target_fps },
    { "set_time_scale", set_time_scale },
    { "reset_clock", reset_clock },
    { "quit", lquit },
    { "set_threads", set_threads },
    { "get_threads", get_threads }, //compat: get_stage_count
    { "get_thread_index", get_thread_index }, //compat: get_stage_id

    { "get_stage_count", get_threads },
    { "get_stage_id", get_thread_index },

    { "init", world_new },
    { "fini", world_fini },
    { "world_info", world_info },
    { "world_stats", world_stats },
    { "dim", dim },

    { "emmy_class", emmy_class },

#define XX(const) { #const, NULL },
    ECS_LUA_ENUMS(XX)
    ECS_LUA_MACROS(XX)
#undef XX
    { NULL, NULL }
};

static void register_types(lua_State *L)
{
    luaL_newmetatable(L, "ecs_type_t");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ecs_ref_t");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ecs_world_t");
    lua_pushcfunction(L, world_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ecs_collect_t");
    lua_pushstring(L, "v");
    lua_setfield(L, -2, "__mode");
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

    luaL_newmetatable(L, "ecs_snapshot_t");
    lua_pushcfunction(L, snapshot_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    luaL_newmetatable(L, "ecs_time_t");
    lua_pushcfunction(L, time__tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pop(L, 1);
}

int luaopen_ecs(lua_State *L)
{
    luaL_checkversion(L);
    luaL_newlibtable(L, ecs_lib);

    ecs_world_t *w;
    int type = lua_type(L, 1);
    int default_world = 0;

    if(type != LUA_TUSERDATA) default_world = 1;

    if(default_world)
    {
        register_types(L);

        w = ecs_lua_get_world(L);

        if(w == NULL) /* Loaded as Lua module */
        {
            w = ecs_init();

            ECS_IMPORT(w, FlecsLua);

            ecs_lua_ctx param = { .L = L, .world = w, .flags = ECS_LUA__DYNAMIC };

            ecs_lua_ctx *ctx = ctx_init(param);

            ecs_singleton_set(w, EcsLuaHost, { L, ctx });
        }

        ecs_world_t **ptr = lua_newuserdata(L, sizeof(ecs_world_t*));
        *ptr = w;

        luaL_setmetatable(L, "ecs_world_t");
        lua_pushvalue(L, -1);
        lua_rawsetp(L, LUA_REGISTRYINDEX, ECS_LUA_DEFAULT_WORLD);
    }
    else /* ecs.init() */
    {
        w = *(ecs_world_t**)lua_touserdata(L, 1);
        lua_pushvalue(L, 1);
    }

    /* registry[world] = { [cursors], [types], [collect], ... } */
    lua_createtable(L, 4, 0);
    lua_pushvalue(L, -1);
    lua_rawsetp(L, LUA_REGISTRYINDEX, w);

        if(default_world) lua_rawgetp(L, LUA_REGISTRYINDEX, ECS_LUA_DEFAULT_CTX);
        else lua_pushvalue(L, 1);

        lua_rawseti(L, -2, ECS_LUA_CONTEXT);

        lua_createtable(L, 128, 0);
        lua_rawseti(L, -2, ECS_LUA_CURSORS);

        lua_createtable(L, 128, 0);
        lua_rawseti(L, -2, ECS_LUA_TYPES);

        /* world[collect] = { [object1], [object2], ... } */
        lua_createtable(L, 0, 16);
        luaL_setmetatable(L, "ecs_collect_t");
        lua_rawseti(L, -2, ECS_LUA_COLLECT);

        lua_createtable(L, 0, 16);
        lua_rawseti(L, -2, ECS_LUA_REGISTRY);

        lua_pushvalue(L, -2); /* world userdata */
        lua_rawseti(L, -2, ECS_LUA_APIWORLD);

    lua_pop(L, 1); /* registry[world] */

    luaL_setfuncs(L, ecs_lib, 1);

#define XX(type) lua_pushinteger(L, ecs_id(Ecs##type)); lua_setfield(L, -2, #type);
    ECS_LUA_TYPEIDS(XX)
#undef XX

#define XX(type) lua_pushinteger(L, ecs_id(ecs_##type##_t)); lua_setfield(L, -2, #type);
    ECS_LUA_PRIMITIVES(XX)
#undef XX

#define XX(const) lua_pushinteger(L, Ecs##const); lua_setfield(L, -2, #const);
    ECS_LUA_BUILTINS(XX)
#undef XX

#define XX(const) lua_pushinteger(L, Ecs##const); lua_setfield(L, -2, #const);
    ECS_LUA_ENUMS(XX)
#undef XX

#define XX(const) lua_pushinteger(L, ECS_##const); lua_setfield(L, -2, #const);
    ECS_LUA_MACROS(XX)
#undef XX

/*
    lua_pushinteger(L, ecs_id(ecs_meta_type_op_kind_t));
    lua_setfield(L, -2, "meta_type_op_kind_t");
*/
    lua_pushinteger(L, ecs_lookup_fullpath(w, "flecs.meta.ecs_meta_type_op_t"));
    lua_setfield(L, -2, "meta_type_op_t");


    return 1;
}

static ecs_lua_ctx *ctx_init(ecs_lua_ctx ctx)
{
    lua_State *L = ctx.L;
    ecs_world_t *world = ctx.world;

    ecs_lua__prolog(L);

    ecs_lua_ctx *default_ctx = ecs_lua_get_context(L, NULL);

    if(default_ctx) return default_ctx;

    ecs_lua_ctx *lctx = lua_newuserdata(L, sizeof(ecs_lua_ctx));

    lua_rawsetp(L, LUA_REGISTRYINDEX, ECS_LUA_DEFAULT_CTX);

    memcpy(lctx, &ctx, sizeof(ecs_lua_ctx));

    lctx->error = 0;
    lctx->progress_ref = LUA_NOREF;
    lctx->prefix_ref = LUA_NOREF;

    if( !(ctx.flags & ECS_LUA__DYNAMIC))
    {
        luaL_requiref(L, "ecs", luaopen_ecs, 1);
        lua_pop(L, 1);
    }

    ecs_lua__epilog(L);

    return lctx;
}

static void ecs_lua_exit(lua_State *L)
{
    if(!L) return;

    ecs_lua_ctx *ctx = ecs_lua_get_context(L, NULL);

    if( !(ctx->internal & ECS_LUA__KEEPOPEN) ) lua_close(L);
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

lua_State *ecs_lua_get_state(ecs_world_t *world)
{
    const EcsLuaHost *host = ecs_singleton_get(world, EcsLuaHost);

    if(!host)
    {
        lua_State *L = lua_newstate(Allocf, NULL);

        ecs_lua_ctx param = { L, world };

        ecs_lua_ctx *ctx = ctx_init(param);

        ecs_singleton_set(world, EcsLuaHost, { L, ctx });

        host = ecs_singleton_get(world, EcsLuaHost);
    }

    ecs_assert(host != NULL, ECS_INTERNAL_ERROR, NULL);
    ecs_assert(host->L != NULL, ECS_INTERNAL_ERROR, NULL);

    return host->L;
}

int ecs_lua_set_state(ecs_world_t *world, lua_State *L)
{
    ecs_assert(L != NULL, ECS_INVALID_PARAMETER, NULL);

    EcsLuaHost *host = ecs_singleton_get_mut(world, EcsLuaHost);

    ecs_lua_exit(host->L);

    ecs_lua_ctx param = { .L = L, .world = world, .internal = ECS_LUA__KEEPOPEN };

    host->L = L;
    host->ctx = ctx_init(param);

    ecs_singleton_modified(world, EcsLuaHost);

    return 0;
}

static ecs_entity_t ecs_id(ecs_meta_type_op_t);

void EcsLuaHost__OnRemove(ecs_iter_t *it)
{
    EcsLuaHost *ptr = ecs_field(it, EcsLuaHost, 1);

    lua_State *L = ptr->L;
    if(L == NULL) return;

    ecs_world_t *wdefault = ecs_lua_get_world(L);

    if(wdefault == it->real_world)
    {/* This is the default world in this VM */
        lua_rawgetp(L, LUA_REGISTRYINDEX, ECS_LUA_DEFAULT_WORLD);
        luaL_callmeta(L, -1, "__gc");

        lua_close(L);
        ptr->L = NULL;
    }
}

void FlecsLuaImport(ecs_world_t *w)
{
    ECS_MODULE(w, FlecsLua);

    ECS_IMPORT(w, FlecsMeta);

    ecs_set_name_prefix(w, "EcsLua");

    ECS_COMPONENT_DEFINE(w, EcsLuaHost);

    ECS_META_COMPONENT(w, EcsLuaWorldInfo);
    ECS_META_COMPONENT(w, EcsLuaGauge);
    ECS_META_COMPONENT(w, EcsLuaGauge_);
    ECS_META_COMPONENT(w, EcsLuaCounter);
    ECS_META_COMPONENT(w, EcsLuaWorldStats);
    ECS_META_COMPONENT(w, EcsLuaTermID);
    ECS_META_COMPONENT(w, EcsLuaTerm);

    ecs_entity_t old_scope = ecs_set_scope(w, 0);

    /* flecs only defines ecs_uptr_t */
    ecs_entity_t e = ecs_entity_init(w, &(ecs_entity_desc_t) { .name = "uintptr_t", .symbol = "uintptr_t" });
    ecs_set(w, e, EcsPrimitive, {.kind = EcsUPtr});

    ecs_set_scope(w, old_scope);

    ecs_set_scope(w, ecs_lookup_fullpath(w, "flecs.meta"));

    //ECS_META_COMPONENT(w, ecs_meta_type_op_t);

    ecs_entity_init(w, &(ecs_entity_desc_t)
    {
        .id = ecs_id(ecs_meta_type_op_t),
        .use_low_id = true,
        .name = "ecs_meta_type_op_t"
    });

    ecs_id(ecs_meta_type_op_t) = ecs_component_init(w, &(ecs_component_desc_t)
    {
        .entity = ecs_id(ecs_meta_type_op_t),
        .type =
        {
            .component = ecs_id(ecs_meta_type_op_t),
            .size = sizeof(ecs_meta_type_op_t),
            .alignment = ECS_ALIGNOF(ecs_meta_type_op_t)
        }
    });

    ecs_struct_init(w, &(ecs_struct_desc_t)
    {
        .entity = ecs_id(ecs_meta_type_op_t),
        .members =
        {
            {.name = (char*)"kind", .type = ecs_id(ecs_i32_t)},
            {.name = (char*)"offset", .type = ecs_id(ecs_i32_t)},
            {.name = (char*)"count", .type = ecs_id(ecs_i32_t)},
            {.name = (char*)"name", .type = ecs_id(ecs_string_t)},
            {.name = (char*)"op_count", .type = ecs_id(ecs_i32_t)},
            {.name = (char*)"size", .type = ecs_id(ecs_i32_t)},
            {.name = (char*)"type", .type = ecs_id(ecs_entity_t)},
            {.name = (char*)"unit", .type = ecs_id(ecs_entity_t)},
        }
    });

    ecs_struct_init(w, &(ecs_struct_desc_t)
    {
        .entity = ecs_id(EcsMetaTypeSerialized),
        .members =
        {
            {.name = (char*)"*ops", .type = ecs_id(EcsVector)},
        }
    });

    ecs_set_scope(w, old_scope);

    /*printf("gauge size %zu\n", sizeof(EcsLuaGauge));
    printf("counter size %zu\n", sizeof(EcsLuaCounter));
    printf("metric size %zu\n", sizeof(ecs_metric_t));
    printf("worldstats: %zu, world_stats: %zu\n", sizeof(EcsWorldStats), sizeof(ecs_world_stats_t));*/

    ecs_assert(sizeof(EcsLuaGauge) == sizeof(ecs_metric_t), ECS_INTERNAL_ERROR, NULL);
    ecs_assert(sizeof(EcsLuaCounter) == sizeof(ecs_metric_t), ECS_INTERNAL_ERROR, NULL);
    ecs_assert(sizeof(EcsLuaWorldStats) == sizeof(ecs_world_stats_t), ECS_INTERNAL_ERROR, NULL);

    ecs_set_hooks(w, EcsLuaHost,
    {
        .ctor = ecs_default_ctor,
        .on_remove = EcsLuaHost__OnRemove // atfini's are called too early (e.g. before OnRemove observers) since at least v3.2.0
    });
}
