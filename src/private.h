#ifndef ECS_LUA__PRIVATE_H
#define ECS_LUA__PRIVATE_H

#include <flecs_lua.h>

#include <lualib.h>
#include <lauxlib.h>


#define ECS_LUA_ENUMS(XX) \
    XX(MatchDefault) \
    XX(MatchAll) \
    XX(MatchAny) \
    XX(MatchExact) \
\
    XX(Module) \
    XX(Prefab) \
    XX(Hidden) \
    XX(Disabled) \
    XX(DisabledIntern) \
    XX(Inactive) \
    XX(OnDemand) \
    XX(Monitor) \
    XX(Pipeline) \
\
    XX(OnAdd) \
    XX(OnRemove) \
\
    XX(OnSet) \
    XX(UnSet) \
\
    XX(PreFrame) \
    XX(OnLoad) \
    XX(PostLoad) \
    XX(PreUpdate) \
    XX(OnUpdate) \
    XX(OnValidate) \
    XX(PostUpdate) \
    XX(PreStore) \
    XX(OnStore) \
    XX(PostFrame) \
\
    XX(Flecs) \
    XX(FlecsCore) \
    XX(World) \
    XX(Singleton) \
    XX(Wildcard)

#define ECS_LUA_MACROS(XX) \
    XX(INSTANCEOF) \
    XX(CHILDOF) \
    XX(TRAIT) \
    XX(AND) \
    XX(OR) \
    XX(XOR) \
    XX(NOT) \
    XX(CASE) \
    XX(SWITCH) \
    XX(OWNED)





#endif /* ECS_LUA__PRIVATE_H */