#include <stdio.h>
#include <flecs.h>
#include <inttypes.h>
#include "src/private.h"

int main(void)
{
#define XX(const) printf("ecs.%s = %d\n", #const, ecs_typeid(Ecs##const));
    ECS_LUA_BUILTINS(XX)
#undef XX

#define XX(const) printf("ecs.%s = %d\n", #const, Ecs##const);
    ECS_LUA_ENUMS(XX)
#undef XX

#define XX(const) printf("ecs.%s = 0x%" PRIX64 "\n", #const, (int64_t)ECS_##const);
    ECS_LUA_MACROS(XX)
#undef XX

    return 0;
}