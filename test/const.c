#include <stdio.h>
#include <flecs.h>
#include "src/private.h"

int main(void)
{
#define XX(const) printf("ecs.%s = %d\n", #const, Ecs##const);
    ECS_LUA_ENUMS(XX)
#undef XX

#define XX(const) printf("ecs.%s = %lld\n", #const, ECS_##const);
    ECS_LUA_MACROS(XX)
#undef XX

    return 0;
}