project('flecs-lua', 'c',
    version : '2.0',
    license : 'mit',
    default_options : 'c_std=c99'
)

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
    'src/ecs.c'
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


if get_option('tests').disabled()
    subdir_done()
endif

test_exe = executable('e', files('test/main.c'), dependencies : flecs_lua_dep)

test_scripts = files(
    'test/entity.lua',
    #'test/meta.lua',
)

foreach s : test_scripts
    test('flecs-lua-test', test_exe, args : s)
endforeach

