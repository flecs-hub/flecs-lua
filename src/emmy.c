#include "private.h"

static const char *primitive_type_name(enum ecs_meta_type_op_kind_t kind)
{
    switch(kind)
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

static const char *array_type_name(const ecs_world_t *world, ecs_meta_type_op_t *op)
{
    return "array_type_name";
    const EcsArray *a = ecs_get(world, op->type, EcsArray);
    ecs_assert(a != NULL, ECS_INTERNAL_ERROR, NULL);

    ecs_entity_t type = a->type;

    //return ecs_get_name(world, type);

    const EcsMetaTypeSerialized *ser = ecs_get(world, type, EcsMetaTypeSerialized);
    ecs_assert(ser != NULL, ECS_INTERNAL_ERROR, NULL);

    ecs_meta_type_op_t *ops = ecs_vec_first(&ser->ops);

#ifndef NDEBUG
    int32_t count = ecs_vec_count(&ser->ops);
    ecs_assert(count >= 2, ECS_INVALID_PARAMETER, NULL);
#endif

    op = &ops[1];

    const char *name = NULL;

    if(op->kind == EcsOpPrimitive) name = primitive_type_name(op->kind);
    else if(op->type) name = ecs_get_name(world, op->type);

    return name;
}

char *ecs_type_to_emmylua(const ecs_world_t *world, ecs_entity_t type, bool struct_as_table)
{
    const EcsMetaTypeSerialized *ser = ecs_get(world, type, EcsMetaTypeSerialized);
    ecs_assert(ser != NULL, ECS_INVALID_PARAMETER, NULL);

    ecs_strbuf_t buf = ECS_STRBUF_INIT;

    const char *class_name = ecs_get_symbol(world, type);
    const char *field_name;
    const char *lua_type = "";
    const char *lua_type_suffix;
    char array_suffix[16];

    if(!class_name) class_name = ecs_get_name(world, type);

    ecs_assert(class_name, ECS_INVALID_PARAMETER, NULL);

    ecs_strbuf_list_append(&buf, "---@class %s\n", class_name);

    ecs_meta_type_op_t *ops = (ecs_meta_type_op_t*)ecs_vec_first(&ser->ops);
    int32_t count = ecs_vec_count(&ser->ops);

    ecs_assert(ops[0].kind == EcsOpPush, ECS_INVALID_PARAMETER, NULL);

    int i, depth = 1;

    for(i=1; i < count; i++)
    {
        ecs_meta_type_op_t *op = &ops[i];

        lua_type_suffix = NULL;

        if(op->count > 1)
        {
            snprintf(array_suffix, sizeof(array_suffix), "[%d]", op->count);
            lua_type_suffix = array_suffix;
        }

        switch(op->kind)
        {
            case EcsOpPush:
            {
                depth++;

                if(depth > 2) continue; /* nested struct */

                if(struct_as_table) lua_type = "table";
                else lua_type = NULL;

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

                if(op->count > 1)
                {
                    snprintf(array_suffix, sizeof(array_suffix), "[%d]", op->count);
                    lua_type_suffix = array_suffix;
                }

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
                lua_type = primitive_type_name(op->kind);
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

        if(!lua_type_suffix) lua_type_suffix = "";

        /* skip nested struct members */
        if(op->kind != EcsOpPush && depth > 1) continue;

        if(op->name) field_name = op->name;
        else field_name = "";

        if(!lua_type && op->type)
        {
            lua_type = ecs_get_symbol(world, op->type);
            if(!lua_type) lua_type = ecs_get_name(world, op->type);
        }

        ecs_strbuf_list_append(&buf, "---@field %s %s%s\n", field_name, lua_type, lua_type_suffix);
    }

    ecs_strbuf_list_append(&buf, "local %s = {}\n", class_name);

    return ecs_strbuf_get(&buf);
}

static const char *kind_str(enum ecs_meta_type_op_kind_t kind)
{
    switch(kind)
    {
        case EcsOpArray: return "EcsOpArray";
        case EcsOpVector: return "EcsOpVector";
        case EcsOpPush: return "EcsOpPush";
        case EcsOpPop: return "EcsOpPop";
        case EcsOpScope: return "EcsOpScope";
        case EcsOpEnum: return "EcsOpEnum";
        case EcsOpBitmask: return "EcsOpBitmask";
        case EcsOpPrimitive: return "EcsOpPrimitive";
        case EcsOpBool: return "EcsOpBool";
        case EcsOpChar: return "EcsOpChar";
        case EcsOpByte: return "EcsOpByte";
        case EcsOpU8: return "EcsOpU8";
        case EcsOpU16: return "EcsOpU16";
        case EcsOpU32: return "EcsOpU32";
        case EcsOpU64: return "EcsOpU64";
        case EcsOpI8: return "EcsOpI8";
        case EcsOpI16: return "EcsOpI16";
        case EcsOpI32: return "EcsOpI32";
        case EcsOpI64: return "EcsOpI64";
        case EcsOpF32: return "EcsOpF32";
        case EcsOpF64: return "EcsOpF64";
        case EcsOpUPtr: return "EcsOpUPtr";
        case EcsOpIPtr: return "EcsOpIPtr";
        case EcsOpString: return "EcsOpString";
        case EcsOpEntity: return "EcsOpEntity";
        default: return "UNKNOWN";
    }
}

static char *str_type_ops(ecs_world_t *w, ecs_entity_t type, int recursive)
{
    const EcsMetaTypeSerialized *ser = ecs_get(w, type, EcsMetaTypeSerialized);

    ecs_strbuf_t buf = ECS_STRBUF_INIT;

    ecs_meta_type_op_t *ops = ecs_vec_first(&ser->ops);
    int count = ecs_vec_count(&ser->ops);

    int i, depth = 0;
    for(i=0; i < count; i++)
    {
        ecs_meta_type_op_t *op = &ops[i];

        ecs_strbuf_append(&buf, "kind: %s\n", kind_str(op->kind));
        ecs_strbuf_append(&buf, "offset: %d\n", op->offset);
        ecs_strbuf_append(&buf, "count: %d\n", op->count);
        ecs_strbuf_append(&buf, "name: \"%s\"\n", op->name ? op->name : "(null)");
        ecs_strbuf_append(&buf, "op_count: %d\n", op->op_count);
        ecs_strbuf_append(&buf, "size: %d\n", op->size);

        const char *name = op->type ? ecs_get_name(w, op->type) : "null";
        ecs_strbuf_append(&buf, "type: %zu (\"%s\")\n", op->type, name ? name : "null");
        //printf("\n");

        if(op->kind == EcsOpPush)
        {
            depth++;
            //if(recursive) print_type_ops(w, type, 1);
        }
        else if(op->kind == EcsOpPop) depth--;
    }

    return ecs_strbuf_get(&buf);
}

int emmy_class(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t type = luaL_checkinteger(L, 1);
    bool b = false;

/*    if(!lua_isnoneornil(L, 2))
    {
        const char *opts = luaL_checkstring(L, 2);
        b = strchr(opts, 't') ? true : false;
    }*/

    if(lua_gettop(L) > 1) b = lua_toboolean(L, 2);

    if(!ecs_get(w, type, EcsMetaTypeSerialized)) luaL_argerror(L, 1, "no metatype for component");
    char *str = str_type_ops(w, type, 0);// ecs_type_to_emmylua(w, type, b);

    lua_pushstring(L, str);

    ecs_os_free(str);

    return 1;
}
