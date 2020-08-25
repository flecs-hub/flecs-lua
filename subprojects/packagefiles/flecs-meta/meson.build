project('flecs-meta', 'c', version : '2.0')

flecs_dep = dependency('flecs')

meta_args = []

if get_option('default_library') == 'static'
    meta_args = '-Dflecs_meta_STATIC'
endif

meta_inc = include_directories('include')

meta_src = files(
    'src/deserializer.c',
    'src/main.c',
    'src/parser.c',
    'src/pretty_print.c',
    'src/serializer.c',
    'src/type.c',
    'src/util.c'
)

meta_lib = library('flecs-meta', 
    meta_src,
    dependencies : flecs_dep,
    include_directories : meta_inc,
    c_args : [ '-Dflecs_meta_EXPORTS', meta_args ],
    implicit_include_directories : false
)

meta_dep = declare_dependency(
    link_with : meta_lib,
    compile_args : meta_args,
    dependencies : flecs_dep,
    include_directories : meta_inc
)

executable('struct',
    'examples/c/01_struct/src/main.c',
    include_directories : 'examples/c/01_struct/include',
    dependencies : meta_dep,
    implicit_include_directories : false
)

if meson.version().version_compare('>= 0.54.0')
    meson.override_dependency('flecs-meta', meta_dep)
endif