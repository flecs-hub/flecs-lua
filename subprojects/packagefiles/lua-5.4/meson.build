project('lua', 'c',
    version : '5.4.2',
    license: 'mit',
    default_options : 'c_std=c99'
)

cc = meson.get_compiler('c')

m_dep = cc.find_library('m', required : false)
dl_dep = cc.find_library('dl', required : false)
readline_dep = cc.find_library('readline', required : false)

lua_deps = [ m_dep, dl_dep ]
lua_args = [ '-DLUA_COMPAT_5_3' ]

if dl_dep.found()
    add_project_arguments('-DLUA_USE_DLOPEN', language : 'c')
endif

if readline_dep.found()
    add_project_arguments('-DLUA_USE_POSIX', language : 'c')
    add_project_arguments('-DLUA_USE_READLINE', language : 'c')
endif

if host_machine.system() == 'windows'
    if get_option('default_library') == 'shared'
        lua_args = '-DLUA_BUILD_AS_DLL'
    endif
endif

lua_inc = include_directories('src')

install_headers('src/lauxlib.h', 'src/lua.h', 'src/lua.hpp', 'src/luaconf.h', 'src/lualib.h')
install_man('doc/lua.1', 'doc/luac.1')

lua_src = files(
    'src/lapi.c',
    'src/lauxlib.c',
    'src/lbaselib.c',
    'src/lcode.c',
    'src/lcorolib.c',
    'src/lctype.c',
    'src/ldblib.c',
    'src/ldebug.c',
    'src/ldo.c',
    'src/ldump.c',
    'src/lfunc.c',
    'src/lgc.c',
    'src/linit.c',
    'src/liolib.c',
    'src/llex.c',
    'src/lmathlib.c',
    'src/lmem.c',
    'src/loadlib.c',
    'src/lobject.c',
    'src/lopcodes.c',
    'src/loslib.c',
    'src/lparser.c',
    'src/lstate.c',
    'src/lstring.c',
    'src/lstrlib.c',
    'src/ltable.c',
    'src/ltablib.c',
    'src/ltm.c',
    'src/lundump.c',
    'src/lutf8lib.c',
    'src/lvm.c',
    'src/lzio.c'
)

lua_lib = library('lua5.4',
    lua_src,
    install : true,
    c_args : lua_args,
    dependencies : lua_deps
)

luac_lib = lua_lib

if get_option('default_library') != 'static'
    luac_lib = static_library('luac_lib', lua_src, dependencies : lua_deps)
endif

lua_dep = declare_dependency(
    link_with : lua_lib,
    compile_args : lua_args,
    dependencies : lua_deps,
    include_directories : lua_inc,
    version : meson.project_version()
)

lua_interpreter = executable('luai',
    'src/lua.c',
    install : true,
    dependencies : [ lua_dep, readline_dep ],
)

lua_compiler = executable('luac',
    'src/luac.c',
    link_with : luac_lib,
    dependencies : lua_deps
)

if meson.version().version_compare('>= 0.54.0')
    meson.override_dependency('lua', lua_dep)
    meson.override_find_program('lua', lua_interpreter)
    meson.override_find_program('luac', lua_compiler)
endif

pkg = import('pkgconfig')

pkg.generate(lua_lib,
    filebase : 'lua5.4',
    description : 'Lua language engine'
)
