#include "test.h"

static int custom_alloc;
static size_t mem_usage;

void *Allocf(void *ud, void *ptr, size_t osize, size_t nsize)
{
    custom_alloc = 1;

    if(!ptr) osize = 0;

    if(!nsize)
    {
        mem_usage -= osize;
        ecs_os_free(ptr);
        return NULL;
    }

    mem_usage += (nsize - osize);
    return ecs_os_realloc(ptr, nsize);
}

static lua_State *new_test_state(void)
{
    lua_State *L = lua_newstate(Allocf, NULL);

    luaL_openlibs(L);

    ecs_assert(custom_alloc == 1, ECS_INTERNAL_ERROR, NULL);

    return L;
}

static void test_abort(void)
{
    fprintf(stderr, "TEST: ecs_os_abort() was called!\n");
    fflush(stdout);
    printf("\n");
}

int main(int argc, char **argv)
{
    if(argc < 2) return 1;

    ecs_os_set_api_defaults();
    ecs_os_api_t os_api = ecs_os_api;
    os_api.abort_ = test_abort;
    ecs_os_set_api(&os_api);

    ecs_world_t *w = ecs_init();

    ECS_IMPORT(w, FlecsLua);

    lua_State *L = ecs_lua_get_state(w);

    ecs_assert(L != NULL, ECS_INTERNAL_ERROR, NULL);

    L = new_test_state();

    lua_newtable(L);

    int i;
    for(i=1; i < argc; i++)
    {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i - 1);
    }

    /* arg = { [0] = "<script_path>", ... } */
    lua_setglobal(L, "arg");

    ecs_lua_set_state(w, L);

    /* The pointer shouldn't change */
    ecs_assert(L == ecs_lua_get_state(w), ECS_INTERNAL_ERROR , NULL);

    luaL_requiref(L, "test", luaopen_test, 0);
    lua_pop(L, 1);

    int ret = luaL_dofile(L, argv[1]);

    if(ret)
    {
        const char *err = lua_tostring(L, lua_gettop(L));
        ecs_os_err("script error: %s\n", err);
    }

    int runs = 2;

    while(ecs_progress(w, 0) && runs)
    {
        ecs_lua_progress(L, 2);

        runs--;
    }

    ecs_fini(w);

    return ret;
}