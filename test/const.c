#include <stdio.h>
#include <flecs.h>
#include <inttypes.h>
#include "src/private.h"

#define ECS_LUA_PRIVATE(XX) \
    XX(LuaGauge) \
    XX(LuaCounter) \
    XX(LuaWorldStats)


int main(void)
{
    const char *dyn = "dynamic";

    printf("local %s = 1\n", dyn);

    printf("\n--Builtin components\n");

#define XX(type) \
    printf("ecs.%s = ", #type); \
    if(ecs_id(Ecs##type)) printf("%" PRIu64 "\n", (uint64_t)ecs_id(Ecs##type)); \
    else printf("%s\n", dyn);

    ECS_LUA_TYPEIDS(XX)
#undef XX

    printf("ecs.type_op_kind_t = %s\n", dyn);
    printf("ecs.type_op_t = %s\n", dyn);


    printf("\n--Builtin primitives\n");

#define XX(type) \
    printf("ecs.%s = ", #type); \
    if(ecs_id(ecs_##type##_t)) printf("%" PRIu64 "\n", (uint64_t)ecs_id(ecs_##type##_t)); \
    else printf("%s\n", dyn);

    ECS_LUA_PRIMITIVES(XX)
#undef XX

    printf("\n--Builtin entities\n");

#define XX(const) printf("ecs.%s = %" PRIu64 "\n", #const, Ecs##const);
    ECS_LUA_BUILTINS(XX)
#undef XX

    printf("\n--Enum values\n");

#define XX(const) printf("ecs.%s = %d\n", #const, Ecs##const);
    ECS_LUA_ENUMS(XX)
#undef XX

    printf("\n--Macros\n");

#define XX(const) printf("ecs.%s = 0x%" PRIX64 "\n", #const, (int64_t)ECS_##const);
    ECS_LUA_MACROS(XX)
#undef XX

#if 0
    ecs_world_t *w = ecs_init();
    FlecsLuaImport(w);

    ecs_entity_t flecs_lua = ecs_lookup(w, "flecs.lua");

    ecs_lookup_child(w, flecs_lua, );
#define XX(const) printf("ecs.%s = 0x%" PRIX64 "\n", #const, (int64_t)ECS_##const);
    ECS_LUA_PRIVATE(XX)
#undef XX
#endif
    return 0;
}