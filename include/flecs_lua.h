#ifndef FLECS_LUA_H
#define FLECS_LUA_H

#include <flecs-lua/bake_config.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef lua_State* ecs_lua_new_cb(void);
typedef void ecs_lua_close_cb(lua_State*);

typedef struct ecs_lua_ctx
{
    lua_State *L;
    ecs_world_t *world;
    ecs_lua_new_cb *new_state_cb;
    ecs_lua_close_cb *close_state_cb;
}ecs_lua_ctx;

FLECS_LUA_EXPORT
int ecs_lua_init(ecs_lua_ctx *ctx);

FLECS_LUA_EXPORT
int ecs_lua_exit(lua_State *L);

/* Get world pointer from registry */
FLECS_LUA_EXPORT
ecs_world_t *ecs_lua_get_world(lua_State *L);

FLECS_LUA_EXPORT
int luaopen_ecs(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif /* FLECS_LUA_H */