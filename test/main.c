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

static void log_msg(int32_t level, const char *file, int32_t line, const char *msg)
{
    FILE *stream;
    if (level >= 0) {
        stream = stdout;
    } else {
        stream = stderr;
    }

    if (level >= 0) {
        if (ecs_os_api.flags_ & EcsOsApiLogWithColors) fputs(ECS_MAGENTA, stream);
        fputs("info", stream);
    } else if (level == -2) {
        if (ecs_os_api.flags_ & EcsOsApiLogWithColors) fputs(ECS_YELLOW, stream);
        fputs("warning", stream);
    } else if (level == -3) {
        if (ecs_os_api.flags_ & EcsOsApiLogWithColors) fputs(ECS_RED, stream);
        fputs("error", stream);
    } else if (level == -4) {
        if (ecs_os_api.flags_ & EcsOsApiLogWithColors) fputs(ECS_RED, stream);
        fputs("fatal", stream);
    }

    if (ecs_os_api.flags_ & EcsOsApiLogWithColors) fputs(ECS_NORMAL, stream);
    fputs(": ", stream);

    if (level >= 0) {
        if (ecs_os_api.log_indent_) {
            char indent[32];
            int i;
            for (i = 0; i < ecs_os_api.log_indent_; i ++) {
                indent[i * 2] = '|';
                indent[i * 2 + 1] = ' ';
            }
            indent[i * 2] = '\0';

            fputs(indent, stream);
        }
    }

    if(level < 0)
    {

        if(file && line) fprintf(stream, "%s(%d): ", file, line);
        else if(file)
        {
            const char *file_ptr = strrchr(file, '/');

            if (!file_ptr) file_ptr = strrchr(file, '\\');

            if (file_ptr) file = file_ptr + 1;

            fputs(file, stream);
            fputs(": ", stream);
        }
        else if(line) fprintf(stream, "(%d): ", line);
    }

    fputs(msg, stream);

    fputs("\n", stream);
}

int main(int argc, char **argv)
{
    if(argc < 2) return 1;

    ecs_os_set_api_defaults();
    ecs_os_api_t os_api = ecs_os_api;
    os_api.abort_ = test_abort;
    os_api.log_ = log_msg;
    ecs_os_set_api(&os_api);

    ecs_log_enable_colors(false); // Colored logs don't work well with file logs

    ecs_world_t *w = ecs_init();

    ECS_IMPORT(w, FlecsMonitor); ecs_warn("FlecsMonitor may impact benchmark results"); // Comment out for benchmarks
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
        ecs_os_err(NULL, 0, err);
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