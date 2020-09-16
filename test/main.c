#include <flecs_lua.h>

#include <lualib.h>
#include <lauxlib.h>

ECS_STRUCT(lua_test_comp,
{
    float blah;
});

ECS_STRUCT(lua_test_struct,
{
    char c;
    char a[4];

    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    int8_t i8;
    int8_t i16;
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;

    lua_test_comp test_comp;
});

ECS_DECLARE_COMPONENT(lua_test_comp);
ECS_DECLARE_COMPONENT(lua_test_struct);

struct vars
{
    lua_test_struct s;
};

static struct vars g;
static struct vars g_out;

static void init_globals(void)
{
    lua_test_struct s =
    {
        .c = 1,
        .u8 = 234,
        .u16 = 234,
        .u32 = 234,
        .u64 = 234,
        .i8 = 234,
        .i16 = 234,
        .i32 = 234,
        .i64 = 234,
        .f32 = 234,
        .f64 = 234,

        .test_comp.blah = 4.0f
    };

    memcpy(&g.s, &s, sizeof(s));
}

int lpush_test_struct(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    lua_pushinteger(L, 232);

 //   ecs_lua_push_ptr(w, ecs_entity(lua_test_struct), &g.s, L);
    return 1;
}

int lset_test_struct(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    return 1;
}

static const luaL_Reg test_lib[] =
{
    { "struct", lpush_test_struct },
    { "set_struct", lset_test_struct },
    { NULL, NULL }
};

int luaopen_test(lua_State *L)
{
    luaL_newlib(L, test_lib);

    return 1;
}

static void init_test_state(lua_State *L)
{
    luaL_openlibs(L);

    luaL_requiref(L, "test", luaopen_test, 1);
    lua_pop(L, 1);
}

static lua_State *new_test_state(void)
{
    lua_State *L = luaL_newstate();

    init_test_state(L);

    return L;
}

static void test_abort(void)
{
    printf("TEST: ecs_os_abort() was called!\n");
    fflush(stdout);
}

static int custom_alloc;

void *Allocf(void *ud, void *ptr, size_t osize, size_t nsize)
{
    custom_alloc = 1;

    if(!nsize)
    {
        ecs_os_free(ptr);
        return NULL;
    }

    return ecs_os_realloc(ptr, nsize);
}

int main(int argc, char **argv)
{
    if(argc < 2) return 1;

    ecs_world_t *w = ecs_init();

    ecs_os_set_api_defaults();
    ecs_os_api_t os_api = ecs_os_api;
    os_api.abort_ = test_abort;
    ecs_os_set_api(&os_api);

    ECS_IMPORT(w, FlecsLua);

    ECS_META(w, lua_test_comp);
    ECS_META(w, lua_test_struct);

    ecs_new_entity(w, 8192, "ecs_lua_test_c_ent", NULL);

    //const EcsLuaHost *ctx = ecs_get(w, EcsSingleton, EcsLuaHost);
    //lua_State *L = ctx->L;

    lua_State *L = lua_newstate(Allocf, NULL);
    ecs_lua_set_state(w, L);
    ecs_assert(custom_alloc, ECS_INTERNAL_ERROR, NULL);

    init_test_state(L);

    int ret = luaL_dofile(L, argv[1]);

    if(ret)
    {
        const char *err = lua_tostring(L, 1);
        printf("script error: %s\n", err);
    }

    ecs_progress(w, 0);
    ecs_progress(w, 0);

    ecs_fini(w);

    return ret;
}