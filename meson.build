project('flecs-lua', 'c',
    version : '2.4.7',
    license : 'MIT',
    default_options : 'c_std=c99'
)

tests = get_option('tests')

if tests.enabled()
    add_global_arguments('-DLUA_USE_APICHECK', language : 'c')
endif

if get_option('lua54') == true
    subproject('lua-54')
endif

flecs_lua_deps = [
    dependency('lua'),
    dependency('flecs'),
    dependency('flecs-meta')
]

flecs_lua_src = []
flecs_lua_args = []

if get_option('default_library') == 'static'
    flecs_lua_args += '-Dflecs_lua_STATIC'
endif

flecs_lua_inc = include_directories('include')

flecs_lua_src += files(
    'src/bulk.c',
    'src/ecs.c',
    'src/emmy.c',
    'src/entity.c',
    'src/hierarchy.c',
    'src/iter.c',
    'src/log.c',
    'src/meta.c',
    'src/misc.c',
    'src/module.c',
    'src/pipeline.c',
    'src/query.c',
    'src/snapshot.c',
    'src/system.c',
    'src/time.c',
    'src/timer.c',
    'src/world.c'
)

flecs_lua_lib = library('flecs-lua',
    flecs_lua_src,
    install : true,
    dependencies : flecs_lua_deps,
    include_directories : flecs_lua_inc,
    c_args : [ '-Dflecs_lua_EXPORTS', flecs_lua_args ],
)

flecs_lua_dep = declare_dependency(
    link_with : flecs_lua_lib,
    dependencies : flecs_lua_deps,
    compile_args : flecs_lua_args,
    include_directories : flecs_lua_inc
)

if meson.version().version_compare('>= 0.54.0')
    meson.override_dependency('flecs-lua', flecs_lua_dep)
endif


if tests.disabled()
    subdir_done()
endif

test_lib = library('test-lua',
    files('test/test.c'),
    dependencies : flecs_lua_dep,
    c_args : [ '-Dflecs_lua_EXPORTS', flecs_lua_args ],
)

test_dep = declare_dependency(
    link_with : test_lib,
    dependencies : flecs_lua_dep,
    compile_args : flecs_lua_args,
)

test_exe = executable('e', files('test/main.c'), dependencies : test_dep)
const_exe = executable('print_const', 'test/const.c', dependencies : flecs_lua_dep)

tests = [
    'misc',
    'entity',
    'meta_limits',
    'meta',
    'snapshot',

    'system',
    'module',
    'pipeline',
    'query',
    'trait',
    'prefab',
    'timer'
]

#Note: Running tests from interpreter requires --layout=flat
env = environment()
lua_path = meson.current_source_dir() / 'test\?.lua'
lua_cpath = test_lib.full_path()
env.set('LUA_PATH', lua_path)
env.set('LUA_CPATH', lua_cpath)

lua_exe = find_program('lua')

foreach name : tests
    script = files('test' / name + '.lua')
    test(name, test_exe, args : script, env : env)
endforeach


run_target('const', command : const_exe)

luac = find_program('luac')
lua = find_program('lua')

run_target('dec',
    command: [
        luac,
        '-l', '-l',
        files('test/script.lua')
    ]
)

run_target('script',
    command: [
        lua,
        files('test/script.lua')
    ]
)

run_target('repl', command: lua)
