#ifndef FLECS_LUA_H
#define FLECS_LUA_H

#include <flecs-lua/bake_config.h>

#ifdef __cplusplus
extern "C" {
#endif

FLECS_LUA_EXPORT
int ecs_lua_init(ecs_world_t *world, lua_State *L);

/* Get world pointer from registry */
FLECS_LUA_EXPORT
ecs_world_t *ecs_lua_get_world(lua_State *L);


#ifdef __cplusplus
}
#endif

#endif /* FLECS_LUA_H */