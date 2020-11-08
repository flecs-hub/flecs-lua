#ifndef FLECS_LUA_H
#define FLECS_LUA_H

#include <flecs-lua/bake_config.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ecs_lua_ctx ecs_lua_ctx;

typedef struct EcsLuaHost
{
    lua_State *L;
    ecs_lua_ctx *ctx;
}EcsLuaHost;

typedef struct FlecsLua
{
    ECS_DECLARE_COMPONENT(EcsLuaHost);
}FlecsLua;

FLECS_LUA_API
void FlecsLuaImport(ecs_world_t *w);

#define FlecsLuaImportHandles(handles)\
    ECS_IMPORT_COMPONENT(handles, EcsLuaHost);\

/* Reinitialize with a custom lua_State */
FLECS_LUA_API
int ecs_lua_set_state(ecs_world_t *w, lua_State *L);

/* Call progress function callback (if set),
   this is meant to be called between iterations. */
FLECS_LUA_API
void ecs_lua_progress(lua_State *L);

/* Pushes the component at ptr onto the stack */
FLECS_LUA_API
void ecs_ptr_to_lua(
    ecs_world_t *world,
    lua_State *L,
    ecs_entity_t type,
    const void *ptr);

/* Converts the Lua value at the given index to the component type */
FLECS_LUA_API
void ecs_lua_to_ptr(
    ecs_world_t *world,
    lua_State *L,
    int idx,
    ecs_entity_t type,
    void *ptr);

/* Pushes the iterator onto the stack */
FLECS_LUA_API
void ecs_iter_to_lua(ecs_iter_t *it, lua_State *L, ecs_type_t select, bool copy);

/* Converts the columns of the iterator at the given index */
FLECS_LUA_API
void ecs_lua_to_iter(ecs_world_t *world, lua_State *L, int idx);

/* Create an EmmyLua class annotation */
char *ecs_type_to_emmylua(ecs_world_t *world, ecs_entity_t type, bool struct_as_table);

/* Get world pointer from registry */
FLECS_LUA_API
ecs_world_t *ecs_lua_get_world(lua_State *L);

FLECS_LUA_API
int luaopen_ecs(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif /* FLECS_LUA_H */