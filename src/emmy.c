#include "private.h"

static const char *primitive_type_name(int primitive)
{
    switch(primitive)
    {
        case EcsBool:
            return "boolean";
        case EcsChar:
        case EcsByte:
        case EcsU8:
        case EcsU16:
        case EcsU32:
        case EcsU64:
        case EcsI8:
        case EcsI16:
        case EcsI32:
        case EcsI64:
        case EcsUPtr:
        case EcsIPtr:
        case EcsEntity:
            return "integer";
        case EcsF32:
        case EcsF64:
            return "number";
        case EcsString:
            return "string";
        default:
            return "unknown";
    }
}

static const char *array_type_name(ecs_world_t *world, ecs_type_op_t *op)
{
    const EcsMetaTypeSerializer *ser = ecs_get_ref_w_entity(world, &op->is.collection, 0, 0);
    ecs_assert(ser != NULL, ECS_INTERNAL_ERROR, NULL);

    ecs_type_op_t *ops = (ecs_type_op_t*)ecs_vector_first(ser->ops, ecs_type_op_t);
    int32_t count = ecs_vector_count(ser->ops);

    op = &ops[1];

    const char *name;

    if(op->kind == EcsOpPrimitive) name = primitive_type_name(op->is.primitive);
    else name = op->name;

    return name;
}

char *ecs_type_to_emmylua(ecs_world_t *world, ecs_entity_t type, bool struct_as_table)
{
    ecs_entity_t ecs_typeid(EcsMetaTypeSerializer) = ecs_lookup_fullpath(world, "flecs.meta.MetaTypeSerializer");
    const EcsMetaTypeSerializer *ser = ecs_get(world, type, EcsMetaTypeSerializer);
    ecs_assert(ser != NULL, ECS_INVALID_PARAMETER, NULL);

    ecs_strbuf_t buf = ECS_STRBUF_INIT;

    const char *class_name = ecs_get_name(world, type);
    const char *field_name;
    const char *lua_type = "";
    const char *lua_type_suffix = "";

    ecs_assert(class_name, ECS_INVALID_PARAMETER, NULL);

    ecs_strbuf_list_append(&buf, "---@class %s\n", class_name);

    ecs_type_op_t *ops = (ecs_type_op_t*)ecs_vector_first(ser->ops, ecs_type_op_t);
    int32_t count = ecs_vector_count(ser->ops);

    ecs_assert(ops[1].kind == EcsOpPush, ECS_INVALID_PARAMETER, NULL);

    int i, depth = 1;

    for(i=2; i < count; i++)
    {
        ecs_type_op_t *op = &ops[i];

        switch(op->kind)
        {
            case EcsOpHeader: continue;
            case EcsOpPush:
            {
                depth++;

                if(struct_as_table) lua_type = "table";
                else lua_type = op->name;

                break;
            }
            case EcsOpPop:
            {
                depth--;
                continue;
            }
            case EcsOpArray:
            {
                lua_type_suffix = "[]";
                lua_type = array_type_name(world, op);
                break;
            }
            case EcsOpVector:
            {
                lua_type_suffix = "[]";
                lua_type = array_type_name(world, op);
                break;
            }
            case EcsOpPrimitive:
            {
                lua_type = primitive_type_name(op->is.primitive);
                break;
            }
            case EcsOpEnum:
            case EcsOpBitmask:
            {
                lua_type = "integer";
                break;
            }
            default: break;
        }

        if(op->kind != EcsOpArray && op->kind != EcsOpVector) lua_type_suffix = "";

        if(op->kind == EcsOpPush) /* struct with name */
        {
            if(depth > 2) continue; /* nested struct */
        }
        else if(depth > 1) continue; /* nested struct members */

        if(op->name) field_name = op->name;
        else field_name = "";

        ecs_strbuf_list_append(&buf, "---@field %s %s%s\n", field_name, lua_type, lua_type_suffix);
    }

    ecs_strbuf_list_append(&buf, "local %s = {}\n", class_name);

    return ecs_strbuf_get(&buf);
}

int emmy_class(lua_State *L)
{
    ecs_world_t *w = ecs_lua_get_world(L);

    ecs_entity_t type = luaL_checkinteger(L, 1);
    bool b = false;

    if(lua_gettop(L) > 1) b = lua_toboolean(L, 2);

    char *str = ecs_type_to_emmylua(w, type, b);

    lua_pushstring(L, str);

    ecs_os_free(str);

    return 1;
}