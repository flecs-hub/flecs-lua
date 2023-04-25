#ifndef FLECS_LUA_TEST_H
#define FLECS_LUA_TEST_H

#include <flecs_lua.h>

#include <lualib.h>
#include <lauxlib.h>

#undef ECS_META_IMPL
#ifndef FLECS_LUA_TEST_IMPL
    #define ECS_META_IMPL EXTERN // Ensure meta symbols are only defined once
#endif

extern ECS_COMPONENT_DECLARE(lua_test_vector);

ECS_STRUCT(lua_test_comp,
{
    float foo;
    uint16_t u16a[4];
});

ECS_STRUCT(lua_test_comp2,
{
    lua_test_comp comp;
    int32_t bar;
});

ECS_ENUM(lua_test_enum,
{
    TestEnum1,
    TestEnum2
});

ECS_BITMASK(lua_test_bitmask,
{
    One = 1,
    Two = 2,
    Three = 3
});

typedef ecs_vec_t lua_test_vector;

//ECS_VECTOR(lua_test_vector,float);

//ECS_MAP(lua_test_mapi32, int32_t, float);

ECS_STRUCT(lua_test_struct,
{
    bool b;
    char c;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;

    char ca[4];

    char *str;
    void *vptr;
    uintptr_t uptr;

    lua_test_enum enumeration;
    lua_test_bitmask bitmask;
    lua_test_vector vector;

    lua_test_comp comp;
    lua_test_comp2 comp2;

    //lua_test_comp sa[3];
});

FLECS_LUA_API void TestImport(ecs_world_t *w);

FLECS_LUA_API int luaopen_test(lua_State *L);

#endif /* FLECS_LUA_TEST_H */