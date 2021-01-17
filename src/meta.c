#include "private.h"

typedef struct ecs_lua_col_t
{
    ecs_entity_t type;
    size_t stride;
    bool readback, update;
    void *ptr;
    const EcsMetaTypeSerializer *ser;
    ecs_meta_cursor_t *cursor;
}ecs_lua_col_t;

typedef struct ecs_lua_each_t
{
    ecs_iter_t *it;
    int32_t i;
    bool from_query, read_prev;
    ecs_lua_col_t cols[];
}ecs_lua_each_t;

static
void serialize_type(
    ecs_world_t *world,
    const ecs_vector_t *ser,
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
        case EcsIPtr:
            lua_pushinteger(L, *(intptr_t*)base);
            break;
        case EcsUPtr:
            lua_pushinteger(L, *(uintptr_t*)base);
            break;
        default:
            luaL_error(L, "unknown primitive (%d)", op->is.primitive);
    }
}

static inline
void serialize_int32(
    ecs_world_t *world,
    ecs_type_op_t *op,
    const void *base,
    lua_State *L)
{
    int32_t value = *(int32_t*)base;
    lua_pushinteger(L, value);
}

static
void serialize_enum(
    ecs_world_t *world,
    ecs_type_op_t *op,
    const void *base,
    lua_State *L)
{
    const EcsEnum *enum_type = ecs_get_ref_w_entity(world, &op->is.constant, 0, 0);
    ecs_assert(enum_type != NULL, ECS_INVALID_PARAMETER, NULL);

    int32_t value = *(int32_t*)base;

    lua_pushinteger(L, value);
}

static
void serialize_bitmask(
    ecs_world_t *world,
    ecs_type_op_t *op,
    const void *base,
    lua_State *L)
{
    const EcsBitmask *bitmask_type = ecs_get_ref_w_entity(world, &op->is.constant, 0, 0);
    ecs_assert(bitmask_type != NULL, ECS_INVALID_PARAMETER, NULL);

    int32_t value = *(int32_t*)base;

    lua_pushinteger(L, value);
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
void serialize_vector(
    ecs_world_t *world,
    ecs_type_op_t *op,
    const void *base,
    lua_State *L)
{
    ecs_vector_t *value = *(ecs_vector_t**)base;

    if(!value)
    {
        lua_pushnil(L);
        return;
    }

    const EcsMetaTypeSerializer *ser = ecs_get_ref_w_entity(world, &op->is.collection, 0, 0);
    ecs_assert(ser != NULL, ECS_INTERNAL_ERROR, NULL);

    int32_t count = ecs_vector_count(value);
    void *array = ecs_vector_first_t(value, op->size, op->alignment);
    ecs_vector_t *elem_ops = ser->ops;

    ecs_type_op_t *elem_op_hdr = (ecs_type_op_t*)ecs_vector_first(elem_ops, ecs_type_op_t);
    ecs_assert(elem_op_hdr != NULL, ECS_INTERNAL_ERROR, NULL);
    ecs_assert(elem_op_hdr->kind == EcsOpHeader, ECS_INTERNAL_ERROR, NULL);
    size_t elem_size = elem_op_hdr->size;

    serialize_elements(world, elem_ops, array, count, elem_size, L);
}

static
void serialize_map(
    ecs_world_t *world,
    ecs_type_op_t *op,
    const void *base,
    lua_State *L)
{
    ecs_map_t *value = *(ecs_map_t**)base;

    const EcsMetaTypeSerializer *key_ser = ecs_get_ref_w_entity(world, &op->is.map.key, 0, 0);
    ecs_assert(key_ser != NULL, ECS_INTERNAL_ERROR, NULL);

    const EcsMetaTypeSerializer *elem_ser = ecs_get_ref_w_entity(world, &op->is.map.element, 0, 0);
    ecs_assert(elem_ser != NULL, ECS_INTERNAL_ERROR, NULL);

    /* 2 instructions, one for the header */
    ecs_assert(ecs_vector_count(key_ser->ops) == 2, ECS_INTERNAL_ERROR, NULL);

    ecs_type_op_t *key_op = ecs_vector_first(key_ser->ops, ecs_type_op_t);
    ecs_assert(key_op->kind == EcsOpHeader, ECS_INTERNAL_ERROR, NULL);
    key_op = &key_op[1];

    ecs_map_iter_t it = ecs_map_iter(value);
    ecs_map_key_t key;
    void *ptr;

    lua_createtable(L, 0, 0);

    while((ptr = _ecs_map_next(&it, 0, &key)))
    {
        serialize_type_op(world, key_op, (void*)&key, L);
        serialize_type(world, elem_ser->ops, ptr, L);

        key = 0;
    }
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
        serialize_int32(world, op, ECS_OFFSET(base, op->offset), L);
        break;
    case EcsOpBitmask:
        serialize_int32(world, op, ECS_OFFSET(base, op->offset), L);
        break;
    case EcsOpArray:
        serialize_array(world, op, ECS_OFFSET(base, op->offset), L);
        break;
    case EcsOpVector:
        serialize_vector(world, op, ECS_OFFSET(base, op->offset), L);
        break;
    case EcsOpMap:
       // serialize_map(world, op, ECS_OFFSET(base, op->offset), L);
        break;
    }
}

static
void serialize_type(
    ecs_world_t *world,
    const ecs_vector_t *ser,
    const void *base,
    lua_State *L)
{
    ecs_assert(base != NULL, ECS_INVALID_PARAMETER, NULL);

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
                if(depth > 1)
                {
                    ecs_assert(op->name != NULL, ECS_INVALID_PARAMETER, NULL);
                    lua_pushstring(L, op->name);
                }
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

static
void update_type(
    ecs_world_t *world,
    const ecs_vector_t *ser,
    const void *base,
    lua_State *L,
    int idx)
{
    ecs_assert(base != NULL, ECS_INVALID_PARAMETER, NULL);

    ecs_type_op_t *ops = (ecs_type_op_t*)ecs_vector_first(ser, ecs_type_op_t);
    int32_t count = ecs_vector_count(ser);

    lua_pushvalue(L, idx);

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
                if(depth > 1)
                {
                    ecs_assert(op->name != NULL, ECS_INVALID_PARAMETER, NULL);
                    int t = lua_getfield(L, -1, op->name);
                    if(t != LUA_TTABLE)
                    {
                        lua_pop(L, 1);
                        lua_newtable(L);
                        lua_pushvalue(L, -1);
                        lua_setfield(L, -3, op->name);
                    }
                }
                break;
            }
            case EcsOpPop:
            {
                if(depth > 1) lua_pop(L, 1);
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

    lua_pop(L, 1);
}

static void deserialize_type(lua_State *L, int idx, ecs_meta_cursor_t *c)
{
    int ktype, vtype, ret, mtype;

    idx = lua_absindex(L, idx);

    vtype = lua_type(L, idx);
    mtype = 0;

    if(vtype == LUA_TTABLE)
    {
        ret = ecs_meta_push(c);
        ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);

        lua_pushnil(L);
    }
    else luaL_checktype(L, idx, LUA_TTABLE);

    while(lua_next(L, idx))
    {
        ktype = lua_type(L, -2);
        vtype = lua_type(L, -1);

        if(!mtype) mtype = ktype;

        if(ktype == LUA_TSTRING)
        {
            const char *key = lua_tostring(L, -2);

            ecs_lua_dbg("move_name field: %s", key);
            ret = ecs_meta_move_name(c, key);

            if(ret) luaL_error(L, "field \"%s\" does not exist", key);
            if(mtype != ktype) luaL_error(L, "table has mixed key types (string key '%s')", key);
        }
        else if(ktype == LUA_TNUMBER)
        {
            lua_Integer key = lua_tointeger(L, -2) - 1;

            ecs_lua_dbg("move idx: %lld", key);
            ret = ecs_meta_move(c, key);

            if(ret) luaL_error(L, "invalid index %I (Lua [%I])", key, key + 1);
            if(mtype != ktype) luaL_error(L, "table has mixed key types (int key [%I]", key+1);
        }
        else luaL_error(L, "invalid key type '%s'", lua_typename(L, ktype));

        switch(vtype)
        {
            case LUA_TTABLE:
            {
                ecs_lua_dbg("meta_push (nested)");
                //ecs_meta_push(c);
                //lua_pushnil(L);
                //depth++;
                int top = lua_gettop(L);
                deserialize_type(L, top, c);
                break;
            }
            case LUA_TNUMBER:
            {
                if(lua_isinteger(L, -1))
                {
                    lua_Integer integer = lua_tointeger(L, -1);

                    ecs_lua_dbg("  set_int: %lld", integer);
                    ret = ecs_meta_set_int(c, integer);

                    if(ret) luaL_error(L, "failed to set integer (%I)", integer);
                }
                else
                {
                    lua_Number number = lua_tonumber(L, -1);

                    ecs_lua_dbg("  set_float %f", number);
                    ret = ecs_meta_set_float(c, number);

                    if(ret) luaL_error(L, "failed to set float (%f)", number);
                }

                break;
            }
            case LUA_TBOOLEAN:
            {
                ecs_lua_dbg("  set_bool: %d", lua_toboolean(L, -1));
                ret = ecs_meta_set_bool(c, lua_toboolean(L, -1));

                ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
                break;
            }
            case LUA_TSTRING:
            {
                ecs_lua_dbg("  set_string: %s", lua_tostring(L, -1));
                ret = ecs_meta_set_string(c, lua_tostring(L, -1));

                ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
                break;
            }
            default:
            {
                if(ktype == LUA_TSTRING)
                    luaL_error(L, "invalid type for field '%s' (got %s)", lua_tostring(L, -2), lua_typename(L, vtype));
                else
                    luaL_error(L, "invalid type at index [%d] (got %s)", lua_tointeger(L, -2), lua_typename(L, vtype));
            }
        }

        lua_pop(L, 1);
    }

    ecs_lua_dbg("meta pop");
    ret = ecs_meta_pop(c);
    ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
}

static
void serialize_column(
    ecs_world_t *world,
    lua_State *L,
    const EcsMetaTypeSerializer *ser,
    const void *base,
    int32_t count)
{
    ecs_vector_t *ops = ser->ops;
    ecs_type_op_t *hdr = ecs_vector_first(ops, ecs_type_op_t);
    ecs_assert(hdr->kind == EcsOpHeader, ECS_INTERNAL_ERROR, NULL);

    serialize_elements(world, ser->ops, base, count, hdr->size, L);
}

static inline ecs_entity_t get_serializer_id(lua_State *L)
{
    int type = lua_rawgetp(L, LUA_REGISTRYINDEX, ECS_LUA_SERIALIZER);
    ecs_assert(type == LUA_TNUMBER, ECS_INTERNAL_ERROR, NULL);
    lua_Integer s = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return s;
}

static const EcsMetaTypeSerializer *get_serializer(lua_State *L, ecs_world_t *world, ecs_entity_t type)
{
    //return ecs_get_w_entity(world, type, get_serializer_id(L));
    int ret = lua_rawgetp(L, LUA_REGISTRYINDEX, world);
    ecs_assert(ret == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    ret = lua_rawgeti(L, -1, ECS_LUA_TYPES);
    ecs_assert(ret == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    ret = lua_rawgeti(L, -1, type);

    ecs_ref_t *ref;

    if(ret != LUA_TNIL)
    {
        ecs_assert(ret == LUA_TUSERDATA, ECS_INTERNAL_ERROR, NULL);

        ref = lua_touserdata(L, -1);

        lua_pop(L, 3);
    }
    else
    {
        lua_pop(L, 1); /* -nil */
        ref = lua_newuserdata(L, sizeof(ecs_ref_t));
        lua_rawseti(L, -2, type);

        *ref = (ecs_ref_t){ .entity = type, .component = get_serializer_id(L) };

        lua_pop(L, 2); /* -types, -world */
    }

    const EcsMetaTypeSerializer *ser = ecs_get_ref_w_entity(world, ref, 0, 0);
    ecs_assert(ser != NULL, ECS_INTERNAL_ERROR, NULL);

    return ser;
}

static int columns__len(lua_State *L)
{
    ecs_iter_t *it = lua_touserdata(L, lua_upvalueindex(1));

    lua_pushinteger(L, it->column_count);

    return 1;
}

static int columns__index(lua_State *L)
{
    ecs_iter_t *it = lua_touserdata(L, lua_upvalueindex(1));
    ecs_world_t *world = it->world;

    lua_Integer i = luaL_checkinteger(L, 2);

    if(i < 1 || i > it->column_count) luaL_argerror(L, 1, "invalid column index");

    if(!it->count)
    {
        lua_pushnil(L);
        return 1;
    }

    ecs_entity_t type = ecs_get_typeid(world, ecs_column_entity(it, i));
    const EcsMetaTypeSerializer *ser = get_serializer(L, world, type);

    if(!ser) luaL_error(L, "column %d cannot be serialized", i);

    lua_settop(L, 1); /* (it.)columns */

    const void *base = ecs_column_w_size(it, 0, i);

    if(!ecs_is_owned(it, i)) serialize_type(world, ser->ops, base, L);
    else serialize_column(world, L, ser, base, it->count);

    lua_pushvalue(L, -1);
    lua_rawseti(L, -3, i);

    return 1;
}

static int entities__index(lua_State *L)
{
    ecs_iter_t *it = ecs_lua__checkiter(L, 1);
    lua_Integer i = luaL_checkinteger(L, 2);

    if(i < 1 || i > it->count)
    {
        if(!it->count) return luaL_error(L, "no matched entities");

        return luaL_error(L, "invalid index (%I)", i, it->count);
    }

    lua_pushinteger(L, it->entities[i-1]);

    return 1;
}

/* expects "it" table at stack top */
static void push_columns(lua_State *L, ecs_iter_t *it)
{
    if(!it->count)
    {
        lua_newtable(L);
        lua_setfield(L, -2, "columns");
        return;
    }

    /* it.columns[] */
    lua_createtable(L, it->column_count, 1);

    /* metatable */
    lua_createtable(L, 0, 2);

    lua_pushlightuserdata(L, it);
    lua_pushcclosure(L, columns__index, 1);
    lua_setfield(L, -2, "__index");

    lua_pushlightuserdata(L, it);
    lua_pushcclosure(L, columns__len, 1);
    lua_setfield(L, -2, "__len");

    lua_setmetatable(L, -2);

    lua_setfield(L, -2, "columns");
}

/* expects "it" table at stack top */
static void push_iter_metadata(lua_State *L, ecs_iter_t *it)
{
    /* it.count */
    lua_pushinteger(L, it->count);
    lua_setfield(L, -2, "count");

    /* it.system */
    lua_pushinteger(L, it->system);
    lua_setfield(L, -2, "system");

    /* it.delta_time */
    lua_pushnumber(L, it->delta_time);
    lua_setfield(L, -2, "delta_time");

    /* it.delta_system_time */
    lua_pushnumber(L, it->delta_system_time);
    lua_setfield(L, -2, "delta_system_time");

    /* it.world_time */
    lua_pushnumber(L, it->world_time);
    lua_setfield(L, -2, "world_time");

    /* it.table_count */
    lua_pushinteger(L, it->table_count);
    lua_setfield(L, -2, "table_count");

    /* it.interrupted_by */
    lua_pushinteger(L, it->interrupted_by);
    lua_setfield(L, -2, "interrupted_by");

    if(it->system)
    {
        ecs_lua_system *sys = it->param;

        if(sys->param_ref >= 0) lua_rawgeti(L, LUA_REGISTRYINDEX, sys->param_ref);
        else lua_pushnil(L);

        lua_setfield(L, -2, "param");
    }

    /* it.entities */
    lua_createtable(L, 0, 1);

    /* metatable */
    lua_createtable(L, 0, 2);

    lua_pushlightuserdata(L, it);
    lua_setfield(L, -2, "__ecs_iter");

    lua_pushcfunction(L, entities__index);
    lua_setfield(L, -2, "__index");

    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "entities");
}

/* expects table at stack top */
static ecs_iter_t *push_iter_metafield(lua_State *L, ecs_iter_t *it, bool copy)
{
    /* metatable */
    lua_createtable(L, 0, 1);

    /* metatable.__ecs_iter = it */
    if(copy)
    {
        ecs_iter_t *ptr = lua_newuserdata(L, sizeof(ecs_iter_t));
        memcpy(ptr, it, sizeof(ecs_iter_t));
        it = ptr;
    }
    else lua_pushlightuserdata(L, it);

    lua_setfield(L, -2, "__ecs_iter");
    lua_setmetatable(L, -2);

    return it;
}

/* Reset with a new base pointer */
static void meta_reset(ecs_meta_cursor_t *cursor, void *base)
{
    ecs_assert(cursor != NULL, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(base != NULL, ECS_INVALID_PARAMETER, NULL);

    cursor->depth = 0;
    cursor->scope[0].start = 1;
    cursor->scope[0].cur_op = 1;
    cursor->scope[0].cur_elem = 0;
    cursor->scope[0].base = base;
    cursor->scope[0].is_collection = false;
    cursor->scope[0].count = 0;
    cursor->scope[0].vector = NULL;
}

static ecs_meta_cursor_t *ecs_lua_cursor(lua_State *L, ecs_world_t *world, ecs_entity_t type, void *base)
{
    int ret = lua_rawgetp(L, LUA_REGISTRYINDEX, world);
    ecs_assert(ret == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    ret = lua_rawgeti(L, -1, ECS_LUA_CURSORS);
    ecs_assert(ret == LUA_TTABLE, ECS_INTERNAL_ERROR, NULL);

    ret = lua_rawgeti(L, -1, type);

    ecs_meta_cursor_t *cursor;

    if(ret == LUA_TUSERDATA)
    {
        cursor = lua_touserdata(L, -1);
        lua_pop(L, 3);

        meta_reset(cursor, base);
    }
    else
    {
        lua_pop(L, 1);
        cursor = lua_newuserdata(L, sizeof(ecs_meta_cursor_t));
        lua_rawseti(L, -2, type);
        lua_pop(L, 2);

        ecs_meta_cursor_t t = ecs_meta_cursor(world, type, base);
        memcpy(cursor, &t, sizeof(ecs_meta_cursor_t));
    }

    return cursor;
}

static
void deserialize_column(
    ecs_world_t *world,
    lua_State *L,
    int idx,
    ecs_entity_t type,
    void *base,
    size_t stride,
    int32_t count)
{
    ecs_meta_cursor_t *c = ecs_lua_cursor(L, world, type, base);

    int j;
    for(j=0; j < count; j++)
    {
        meta_reset(c, (char*)base + j * stride);

        lua_rawgeti(L, idx, j + 1); /* columns[i+1][j+1] */
        deserialize_type(L, -1, c);

        lua_pop(L, 1);
    }
}

void ecs_ptr_to_lua(
    ecs_world_t *world,
    lua_State *L,
    ecs_entity_t type,
    const void *ptr)
{
    const EcsMetaTypeSerializer *ser = get_serializer(L, world, type);

    serialize_type(world, ser->ops, ptr, L);
}

void ecs_lua_to_ptr(
    ecs_world_t *world,
    lua_State *L,
    int idx,
    ecs_entity_t type,
    void *ptr)
{
    ecs_meta_cursor_t *c = ecs_lua_cursor(L, world, type, ptr);

    deserialize_type(L, idx, c);
}

void ecs_lua_type_update(
    ecs_world_t *world,
    lua_State *L,
    int idx,
    ecs_entity_t type,
    void *ptr)
{
    const EcsMetaTypeSerializer *ser = get_serializer(L, world, type);

    update_type(world, ser->ops, ptr, L, idx);
}

ecs_iter_t *ecs_iter_to_lua(ecs_iter_t *it, lua_State *L, bool copy)
{
    /* it */
    lua_createtable(L, 0, 16);

    /* metatable.__ecs_iter */
    it = push_iter_metafield(L, it, copy);

    push_iter_metadata(L, it);
    push_columns(L, it);

    return it;
}

ecs_iter_t *ecs_lua_to_iter(lua_State *L, int idx)
{
    ecs_os_dbg("ECS_LUA_TO_ITER");
    ecs_lua__prolog(L);
    ecs_iter_t *it = ecs_lua__checkiter(L, idx);
    ecs_world_t *world = it->world;

    if(lua_getfield(L, idx, "interrupted_by") == LUA_TNUMBER) it->interrupted_by = lua_tointeger(L, -1);

    lua_pop(L, 1);

    /* newly-returned iterators have it->count = 0 */
    if(!it->count) return it;

    luaL_getsubtable(L, idx, "columns");
    luaL_checktype(L, -1, LUA_TTABLE);

    int32_t i;
    for(i=1; i <= it->column_count; i++)
    {
        if(it->query && ecs_is_readonly(it, i)) continue;

        int type = lua_rawgeti(L, -1, i); /* columns[i] */
        bool is_owned = ecs_is_owned(it, i);

        if(type == LUA_TNIL)
        {
            ecs_lua_dbg("skipping empty column %d (not serialized?)", i);
            lua_pop(L, 1);
            continue;
        }

        if(is_owned) { ecs_assert(it->count == lua_rawlen(L, -1), ECS_INTERNAL_ERROR, NULL); }

        int32_t count = it->count;
        ecs_entity_t column_entity = ecs_get_typeid(world, ecs_column_entity(it, i));
        void *base = ecs_column_w_size(it, 0, i);

        if(!is_owned) ecs_lua_to_ptr(world, L, -1, column_entity, base);
        else deserialize_column(world, L, -1, column_entity, base, ecs_column_size(it, i), count);

        lua_pop(L, 1); /* columns[i] */
    }

    lua_pop(L, 1); /* columns */

    ecs_lua__epilog(L);

    return it;
}

void ecs_lua_iter_update(lua_State *L, int idx, ecs_iter_t *it)
{
    lua_pushvalue(L, idx);

    push_iter_metadata(L, it);

    lua_pushnil(L);
    lua_setfield(L, -2, "columns");

    push_columns(L, it);

    lua_pop(L, 1);
}

/* Progress the query iterator at the given index */
bool ecs_lua_query_next(lua_State *L, int idx)
{ecs_os_dbg("QUERY_NEXT");
    ecs_iter_t *it = ecs_lua_to_iter(L, idx);

    if(!ecs_query_next(it)) return false;

    ecs_lua_iter_update(L, idx, it);

    return true;
}

static void each_reset_columns(lua_State *L, ecs_lua_each_t *each)
{
    ecs_iter_t *it = each->it;
    ecs_lua_col_t *col = each->cols;

    each->i = 0;

    if(!it->count) return;

    int i;
    for(i=1; i <= it->column_count; i++, col++)
    {
        col->type = ecs_get_typeid(it->world, ecs_column_entity(it, i));
        col->stride = ecs_column_size(it, i);
        col->ptr = ecs_column_w_size(it, 0, i);
        col->ser = get_serializer(L, it->world, col->type);
        col->cursor = ecs_lua_cursor(L, it->world, col->type, col->ptr);

        if(!ecs_is_owned(it, 1)) col->stride = 0;

        if(it->query && ecs_is_readonly(it, i)) col->readback = false;
        else col->readback = true;

        col->update = true;
    }
}

static int next_func(lua_State *L)
{
    ecs_lua_each_t *each = lua_touserdata(L, lua_upvalueindex(1));
    ecs_lua_col_t *col = each->cols;
    ecs_iter_t *it = each->it;
    int idx, j, i = each->i;
    bool end = false;
    void *ptr;

    ecs_lua_dbg("each() i: %d", i);

    if(!each->read_prev) goto skip_readback;

    for(j=0; j < it->column_count; j++, col++)
    {
        if(!col->readback) continue;

        ecs_lua_dbg("each() readback: %d", i-1);

        idx = lua_upvalueindex(j+2);
        ptr = ECS_OFFSET(col->ptr, col->stride * (i - 1));

        meta_reset(col->cursor, ptr);
        deserialize_type(L, idx, col->cursor);
    }

    col = each->cols;

skip_readback:

    each->read_prev = true;

    if(i == it->count)
    {
        if(each->from_query)
        {
            if(ecs_lua_query_next(L, 1)) each_reset_columns(L, each);
            else end = true;
        }
        else end = true;
    }

    if(end) return 0;

    for(j=0; j < it->column_count; j++, col++)
    {// optimization: shared components should be read back at the end
        if(!col->update) continue;

        idx = lua_upvalueindex(j+2);
        ptr = ECS_OFFSET(col->ptr, col->stride * i);

        lua_pushvalue(L, idx);
        update_type(it->world, col->ser->ops, ptr, L, idx);
    }

    lua_pushinteger(L, it->entities[i]);

    each->i++;

    return it->column_count + 1;
}

int each_func(lua_State *L)
{ecs_lua_dbg("ecs.each()");
    ecs_query_t *q = NULL;
    ecs_iter_t *it;
    int iter_idx = 1;

    if(lua_type(L, 1) == LUA_TUSERDATA)
    {
        q = checkquery(L, 1);
        ecs_iter_t iter = ecs_query_iter(q);
        ecs_query_next(&iter);
        it = ecs_iter_to_lua(&iter, L, true);
        iter_idx = lua_gettop(L);
    }
    else it = ecs_lua__checkiter(L, 1);

    size_t size = sizeof(ecs_lua_each_t) + it->column_count * sizeof(ecs_lua_col_t);
    ecs_lua_each_t *each = lua_newuserdata(L, size);

    each->it = it;
    each->from_query = q ? true : false;
    each->read_prev = false;

    each_reset_columns(L, each);

    int i;
    for(i=0; i < it->column_count; i++)
    {
        lua_newtable(L);
    }

    lua_pushcclosure(L, next_func, it->column_count + 1);

    /* it */
    lua_pushvalue(L, iter_idx);

    lua_pushinteger(L, 1);

    return 3;
}
