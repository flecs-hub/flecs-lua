#include "test.h"

ECS_DECLARE_COMPONENT(lua_test_comp);
ECS_DECLARE_COMPONENT(lua_test_comp2);
ECS_DECLARE_COMPONENT(lua_test_struct);

struct vars
{
    lua_test_struct s;
};

static struct vars g;

#define TEST_COMP_INIT { .foo = 123.4f, .u16a = 123, 4321, 32, 688}

static void init_globals(void)
{
    lua_test_struct s =
    {
        .b = true,
        .c = 1,
        .u8 = 2,
        .u16 = 4,
        .u32 = 8,
        .u64 = 16,
        .i8 = 32,
        .i16 = 64,
        .i32 = 128,
        .i64 = 256,
        .f32 = 512,
        .f64 = 1024,

        .ca = { 10, 20, 30, 40 },

        .str = "test string",
        .vptr = &g,
        .uptr = (uintptr_t)&g,

        .enumeration = 465,
        .bitmask = One | Two,

        .comp.foo = 4.0f,
        .comp.u16a = { 10, 20, 30, 40 },
        .comp2.bar = 5,
        .comp2.comp.u16a = { 100, 200, 300, 400 }
    };

    memcpy(&g.s, &s, sizeof(s));
}

static const luaL_Reg test_lib[] =
{
    { "dummy", NULL },
    { NULL, NULL }
};

ECS_CTOR(lua_test_struct, ptr,
{
    ptr->str = NULL;
});

ECS_DTOR(lua_test_struct, ptr,
{
    ecs_os_free(ptr->str);
    ptr->str = NULL;
});

ECS_COPY(lua_test_struct, dst, src,
{
    if(dst->str)
    {
        ecs_os_free(dst->str);
        dst->str = NULL;
    }

    if(src->str) dst->str = ecs_os_strdup(src->str);
    else dst->str = NULL;

    void *t = dst->str;
    memcpy(dst, src, sizeof(lua_test_struct));
    dst->str = t;
});

void TestImport(ecs_world_t *w)
{
    ECS_MODULE(w, Test);

    ECS_IMPORT(w, FlecsMeta);

    ecs_entity_t scope = ecs_set_scope(w, 0);

    init_globals();

    ECS_META(w, lua_test_comp);
    ECS_META(w, lua_test_comp2);
    ECS_META(w, lua_test_enum);
    ECS_META(w, lua_test_bitmask);
    ECS_META(w, lua_test_vector);
    ECS_META(w, lua_test_mapi32);
    ECS_META(w, lua_test_struct);

    ecs_set_component_actions(w, lua_test_struct,
    {
        .ctor = ecs_ctor(lua_test_struct),
        .dtor = ecs_dtor(lua_test_struct),
        .copy = ecs_copy(lua_test_struct),
    });

    ecs_entity_init(w, &(ecs_entity_desc_t)
    {
        .entity = 8192,
        .name = "ecs_lua_test_c_ent"
    });

    ecs_set(w, 0, lua_test_comp, TEST_COMP_INIT);
    ecs_set(w, 0, lua_test_comp, TEST_COMP_INIT);
    ecs_set(w, 0, lua_test_comp, TEST_COMP_INIT);

    ecs_set_id(w, ecs_id(lua_test_struct), ecs_id(lua_test_struct), sizeof(lua_test_struct), &g.s);

    ecs_set_scope(w, scope);
}

int luaopen_test(lua_State *L)
{
    luaL_requiref(L, "ecs", luaopen_ecs, 0);
    lua_pop(L, 1);

    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_assert(w != NULL, ECS_INTERNAL_ERROR, NULL);

    ECS_IMPORT(w, Test);

    luaL_newlib(L, test_lib);

    return 1;
}
