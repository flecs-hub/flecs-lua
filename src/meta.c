#include "private.h"

static
void serialize_type(
    ecs_world_t *world,
    ecs_vector_t *ser,
    const void *base,
    lua_State *L);

static
void serialize_type_op(
    ecs_world_t *world,
    ecs_type_op_t *op,
    const void *base,
    lua_State *L);

static
void serialize_primitive(
    ecs_world_t *world,
    ecs_type_op_t *op,
    const void *base,
    lua_State *L)
{
    switch(op->is.primitive)
    {
        case EcsBool:
            lua_pushboolean(L, (int)*(bool*)base);
            break;
        case EcsChar:
            lua_pushinteger(L, *(char*)base);
            break;
        case EcsString:
            lua_pushstring(L, *(char**)base);
            break;
        case EcsByte:
            lua_pushinteger(L, *(uint8_t*)base);
            break;
        case EcsU8:
            lua_pushinteger(L, *(uint8_t*)base);
            break;
        case EcsU16:
            lua_pushinteger(L, *(uint16_t*)base);
            break;
        case EcsU32:
            lua_pushinteger(L, *(uint32_t*)base);
            break;
        case EcsU64:
            lua_pushinteger(L, *(uint64_t*)base);
            break;
        case EcsI8:
            lua_pushinteger(L, *(int8_t*)base);
            break;
        case EcsI16:
            lua_pushinteger(L, *(int16_t*)base);
            break;
        case EcsI32:
            lua_pushinteger(L, *(int32_t*)base);
            break;
        case EcsI64:
            lua_pushinteger(L, *(int64_t*)base);
            break;
        case EcsF32:
            lua_pushnumber(L, *(float*)base);
            break;
        case EcsF64:
            lua_pushnumber(L, *(double*)base);
            break;
        case EcsEntity:
            lua_pushinteger(L, *(ecs_entity_t*)base);
            break;
    }
}

static
void serialize_elements(
    ecs_world_t *world,
    ecs_vector_t *elem_ops,
    const void *base,
    int32_t elem_count,
    int32_t elem_size,
    lua_State *L)
{
    const void *ptr = base;

    lua_createtable(L, elem_count, 0);

    int i;
    for(i=0; i < elem_count; i++)
    {
        serialize_type(world, elem_ops, ptr, L);
        lua_rawseti(L, -2, i + 1);

        ptr = ECS_OFFSET(ptr, elem_size);
    }
}

static
void serialize_array(
    ecs_world_t *world,
    ecs_type_op_t *op,
    const void *base,
    lua_State *L)
{
    const EcsMetaTypeSerializer *ser = ecs_get_ref_w_entity(world, &op->is.collection, 0, 0);
    ecs_assert(ser != NULL, ECS_INTERNAL_ERROR, NULL);

    serialize_elements(world, ser->ops, base, op->count, op->size, L);
}

static
void serialize_type_op(
    ecs_world_t *world,
    ecs_type_op_t *op,
    const void *base,
    lua_State *L)
{
    switch(op->kind)
    {
    case EcsOpHeader:
    case EcsOpPush:
    case EcsOpPop:
        ecs_abort(ECS_INVALID_PARAMETER, NULL);
        break;
    case EcsOpPrimitive:
        serialize_primitive(world, op, ECS_OFFSET(base, op->offset), L);
        break;
    case EcsOpEnum:
      // ecs_lua_push_enum(world, op, ECS_OFFSET(base, op->offset), L);
        break;
    case EcsOpBitmask:
      // ecs_lua_push_bitmask(world, op, ECS_OFFSET(base, op->offset), L);
        break;
    case EcsOpArray:
        serialize_array(world, op, ECS_OFFSET(base, op->offset), L);
        break;
    case EcsOpVector:
        //ecs_lua_push_vector(world, op, ECS_OFFSET(base, op->offset), L);
        break;
    case EcsOpMap:
       // ecs_lua_push_map(world, op, ECS_OFFSET(base, op->offset), L);
        break;
    }
}

static
void serialize_type(
    ecs_world_t *world,
    ecs_vector_t *ser,
    const void *base,
    lua_State *L)
{
    ecs_type_op_t *ops = (ecs_type_op_t*)ecs_vector_first(ser, ecs_type_op_t);
    int32_t count = ecs_vector_count(ser);

    int i, depth = 0;

    for(i=0; i < count; i++)
    {
        ecs_type_op_t *op = &ops[i];

        switch(op->kind)
        {
            case EcsOpHeader: break;
            case EcsOpPush:
            {
                depth++;
                if(depth > 1) lua_pushstring(L, op->name);
                lua_newtable(L);
                break;
            }
            case EcsOpPop:
            {
                if(depth > 1) lua_settable(L, -3);
                depth--;
                break;
            }
            default:
            {
                serialize_type_op(world, op, base, L);
                if(op->name) lua_setfield(L, -2, op->name);
                break;
            }
        }
    }
}

void ecs_lua_push_ptr(
    ecs_world_t *world,
    lua_State *L,
    ecs_entity_t type,
    const void *ptr)
{
    ecs_entity_t ecs_entity(EcsMetaTypeSerializer) = ecs_lookup_fullpath(world, "flecs.meta.MetaTypeSerializer");
    const EcsMetaTypeSerializer *ser = ecs_get(world, type, EcsMetaTypeSerializer);
    ecs_assert(ser != NULL, ECS_INVALID_PARAMETER, NULL);

    serialize_type(world, ser->ops, ptr, L);
}

static void deserialize_type(ecs_world_t *world, ecs_meta_cursor_t *c, lua_State *L, int idx)
{
    int ktype, vtype, ret, depth = 0;

    ecs_meta_push(c);

    luaL_checktype(L, idx, LUA_TTABLE);

    lua_pushnil(L);

    while(lua_next(L, idx))
    {
        ktype = lua_type(L, -2);
        vtype = lua_type(L, -1);

        switch(ktype)
        {
            case LUA_TSTRING:
            {
                ecs_os_dbg("move_name field: %s", lua_tostring(L, -2));
                ret = ecs_meta_move_name(c, lua_tostring(L, -2));
                ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
                break;
            }
            case LUA_TNUMBER:
            {
                ecs_os_dbg("move idx: %lld", lua_tointeger(L, -2)-1);
                ret = ecs_meta_move(c, lua_tointeger(L, -2)-1);
                ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
                break;
            }
            default: /* shouldn't happen */
            {
                ecs_abort(ECS_INTERNAL_ERROR, NULL);
            }
        }

        switch(vtype)
        {
            case LUA_TTABLE:
            {
             //   lua_pop(L, 1);
              //  continue;
                ecs_os_dbg("meta_push (nested)\n");
                //ecs_meta_push(c);
                //lua_gettable(L, -2);
                //lua_pushnil(L);
                //idx = -2;
                deserialize_type(world, c, L, lua_gettop(L));
                //lua_pop(L, 1);
                break;
            }
            case LUA_TNUMBER:
            {
                if(lua_isinteger(L, -1))
                {
                    ecs_os_dbg("  set_int: %lld", lua_tointeger(L, -1));
                    int ret = ecs_meta_set_int(c, lua_tointeger(L, -1));
                    ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
                }
                else
                {
                    ecs_os_dbg("  set_float %f", lua_tonumber(L, -1));
                    ret = ecs_meta_set_float(c, lua_tonumber(L, -1));
                    ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
                }

                break;
            }
            case LUA_TBOOLEAN:
            {
                ecs_os_dbg("  set_bool: %d", lua_toboolean(L, -1));
                int ret = ecs_meta_set_bool(c, lua_toboolean(L, -1));
                ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
                break;
            }
            case LUA_TSTRING:
            {
                ecs_os_dbg("  set_string: %s", lua_tostring(L, -2));
                ret = ecs_meta_set_string(c, lua_tostring(L, -1));
                ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
                break;
            }
            case LUA_TNIL:
            {
                ecs_os_dbg("  set_null");
                ret = ecs_meta_set_null(c);
                ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
                break;
            }
        }

        lua_pop(L, 1);
    }

    ecs_os_dbg("meta pop");
    ret = ecs_meta_pop(c);
    ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
}

void ecs_lua_to_ptr(
    ecs_world_t *world,
    lua_State *L,
    int idx,
    ecs_entity_t type,
    void *ptr)
{
    ecs_meta_cursor_t c = ecs_meta_cursor(world, type, ptr);

    deserialize_type(world, &c, L, idx);
}
