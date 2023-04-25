#include "private.h"

#include <ctype.h> /* tolower() */

typedef struct ecs_lua_col_t
{
    ecs_entity_t type;
    size_t stride;
    bool readback, update;
    void *ptr;
    const EcsMetaTypeSerialized *ser;
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
void serialize_type_op(
    const ecs_world_t *world,
    ecs_meta_type_op_t *op,
    const void *base,
    lua_State *L);

static
void serialize_type(
    const ecs_world_t *world,
    const ecs_vec_t *v_ops,
    const void *base,
    lua_State *L);

static
void serialize_elements(
    const ecs_world_t *world,
    ecs_meta_type_op_t *ops,
    int32_t op_count,
    const void *base,
    int32_t elem_count,
    int32_t elem_size,
    lua_State *L);

static
void serialize_type_ops(
    const ecs_world_t *world,
    ecs_meta_type_op_t *ops,
    int32_t op_count,
    const void *base,
    int32_t in_array,
    lua_State *L)
{
    int i, depth = 0;

    for(i=0; i < op_count; i++)
    {
        ecs_meta_type_op_t *op = &ops[i];

        if(in_array <= 0)
        {
           int32_t elem_count  = op->count;

            if(elem_count > 1)
            {/* serialize inline array */
                serialize_elements(world, op, op->op_count, base, op->count, op->size, L);

                if(op->name) lua_setfield(L, -2, op->name);

                i += op->op_count - 1;
                continue;
            }
        }

        switch(op->kind)
        {
            case EcsOpPush:
            {
                depth++;
                in_array--;

                if(depth > 1)
                {
                    ecs_assert(op->name != NULL, ECS_INVALID_PARAMETER, NULL);
                    lua_pushstring(L, op->name);
                }

                int32_t member_count = ecs_map_count(&op->members->impl);
                lua_createtable(L, 0, member_count);
                break;
            }
            case EcsOpPop:
            {
                if(depth > 1) lua_settable(L, -3);

                depth--;
                in_array++;
                break;
            }
            default:
            {
                serialize_type_op(world, op, base, L);

                if(op->name && (in_array <= 0))
                {
                    lua_setfield(L, -2, op->name);
                }

                break;
            }
        }
    }
}

static
void serialize_primitive(
    const ecs_world_t *world,
    ecs_meta_type_op_t *op,
    const void *base,
    lua_State *L)
{
    switch(op->kind)
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
            luaL_error(L, "unknown primitive (%d)", op->kind);
    }
}

static
void serialize_elements(
    const ecs_world_t *world,
    ecs_meta_type_op_t *ops,
    int32_t op_count,
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
        serialize_type_ops(world, ops, op_count, ptr, 1, L);
        lua_rawseti(L, -2, i + 1);

        ptr = ECS_OFFSET(ptr, elem_size);
    }
}

static
void serialize_type_elements(
    const ecs_world_t *world,
    ecs_entity_t type,
    const void *base,
    int32_t elem_count,
    lua_State *L)
{
    const EcsMetaTypeSerialized *ser = ecs_get(world, type, EcsMetaTypeSerialized);
    ecs_assert(ser != NULL, ECS_INTERNAL_ERROR, NULL);

    const EcsComponent *comp = ecs_get(world, type, EcsComponent);
    ecs_assert(comp != NULL, ECS_INTERNAL_ERROR, NULL);

    ecs_meta_type_op_t *ops = ecs_vec_first(&ser->ops);
    int32_t op_count = ecs_vec_count(&ser->ops);

    serialize_elements(world, ops, op_count, base, elem_count, comp->size, L);
}

static
void serialize_array(
    const ecs_world_t *world,
    ecs_meta_type_op_t *op,
    const void *base,
    lua_State *L)
{
    const EcsArray *a = ecs_get(world, op->type, EcsArray);
    ecs_assert(a != NULL, ECS_INTERNAL_ERROR, NULL);

    serialize_type_elements(world, a->type, base, op->count, L);
}

static
void serialize_vector(
    const ecs_world_t *world,
    ecs_meta_type_op_t *op,
    const void *base,
    lua_State *L)
{
    ecs_vec_t *value = *(ecs_vec_t**)base;

    if(!value)
    {
        lua_pushnil(L);
        return;
    }

    const EcsVector *v = ecs_get(world, op->type, EcsVector);
    ecs_assert(v != NULL, ECS_INTERNAL_ERROR, NULL);

    const EcsComponent *comp = ecs_get(world, v->type, EcsComponent);
    ecs_assert(comp != NULL, ECS_INTERNAL_ERROR, NULL);

    int32_t count = ecs_vec_count(value);
    void *array = ecs_vec_first(value);

    serialize_type_elements(world, v->type, array, count, L);
}

static
void serialize_type_op(
    const ecs_world_t *world,
    ecs_meta_type_op_t *op,
    const void *base,
    lua_State *L)
{
    switch(op->kind)
    {
    case EcsOpPush:
    case EcsOpPop:
        ecs_abort(ECS_INVALID_PARAMETER, NULL);
        break;

    case EcsOpEnum:
    case EcsOpBitmask:
        lua_pushinteger(L, *(int32_t*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpArray:
        serialize_array(world, op, ECS_OFFSET(base, op->offset), L);
        break;
    case EcsOpVector:
        serialize_vector(world, op, ECS_OFFSET(base, op->offset), L);
        break;
    //case EcsOpMap:
       // serialize_map(world, op, ECS_OFFSET(base, op->offset), L);
        //break;

    case EcsOpPrimitive:
        serialize_primitive(world, op, ECS_OFFSET(base, op->offset), L);
        break;
//////////////////////////
    case EcsOpBool:
        lua_pushboolean(L, (int)*(bool*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpChar:
        lua_pushinteger(L, *(char*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpByte:
        lua_pushinteger(L, *(uint8_t*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpU8:
        lua_pushinteger(L, *(uint8_t*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpU16:
        lua_pushinteger(L, *(uint16_t*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpU32:
        lua_pushinteger(L, *(uint32_t*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpU64:
        lua_pushinteger(L, *(uint64_t*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpI8:
        lua_pushinteger(L, *(int8_t*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpI16:
        lua_pushinteger(L, *(int16_t*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpI32:
        lua_pushinteger(L, *(int32_t*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpI64:
        lua_pushinteger(L, *(int64_t*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpF32:
        lua_pushnumber(L, *(float*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpF64:
        lua_pushnumber(L, *(double*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpEntity:
        lua_pushinteger(L, *(ecs_entity_t*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpIPtr:
        lua_pushinteger(L, *(intptr_t*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpUPtr:
        lua_pushinteger(L, *(uintptr_t*)ECS_OFFSET(base, op->offset));
        break;
    case EcsOpString:
        lua_pushstring(L, *(char**)ECS_OFFSET(base, op->offset));
        break;
    }
}

static
void serialize_constants(
    ecs_world_t *world,
    ecs_meta_type_op_t *op,
    lua_State *L,
    const char *prefix,
    bool lowercase)
{
    const ecs_map_t *map = NULL;
    const EcsEnum *enum_type = NULL;
    const EcsBitmask *bitmask_type = NULL;

    enum_type = ecs_get(world, op->type, EcsEnum);

    if(enum_type != NULL)
    {
        map = &enum_type->constants;
    }
    else
    {
        bitmask_type = ecs_get(world, op->type, EcsBitmask);
        ecs_assert(bitmask_type != NULL, ECS_INVALID_PARAMETER, NULL);

        map = &bitmask_type->constants;
    }

    void *ptr;
    ecs_map_key_t key;
    ecs_map_iter_t it = ecs_map_iter(map);

    const char *name;
    lua_Integer value;

    while(ecs_map_next(&it))
    {
        ptr = ecs_map_ptr(&it);

        if(enum_type != NULL)
        {
            ecs_enum_constant_t *constant = ptr;
            name = constant->name;
            value = constant->value;
        }
        else
        {
            ecs_bitmask_constant_t *constant = ptr;
            name = constant->name;
            value = constant->value;
        }

        if(prefix)
        {
            char *ptr = strstr(name, prefix);
            size_t len = strlen(prefix);

            if(ptr == name && ptr[len] != '\0')
            {
                name = ptr + len;
            }
        }

        if(lowercase)
        {
            char *p = ecs_os_strdup(name);

            int i;
            for(i=0; p[i] != '\0'; i++)
            {
                p[i] = tolower(p[i]);
            }

            lua_pushstring(L, p);

            ecs_os_free(p);
        }
        else lua_pushstring(L, name);

        lua_pushinteger(L, value);

        lua_settable(L, -3);
    }
}

static
void serialize_type(
    const ecs_world_t *world,
    const ecs_vec_t *v_ops,
    const void *base,
    lua_State *L)
{
    ecs_assert(base != NULL, ECS_INVALID_PARAMETER, NULL);

    ecs_meta_type_op_t *ops = ecs_vec_first(v_ops);
    int32_t count = ecs_vec_count(v_ops);

    serialize_type_ops(world, ops, count, base, 0, L);
}

static
void update_type(
    const ecs_world_t *world,
    const ecs_vec_t *ser,
    const void *base,
    lua_State *L,
    int idx)
{
    ecs_assert(base != NULL, ECS_INVALID_PARAMETER, NULL);

    ecs_meta_type_op_t *ops = (ecs_meta_type_op_t*)ecs_vec_first(ser);
    int32_t count = ecs_vec_count(ser);

    lua_pushvalue(L, idx);

    int i, depth = 0;

    for(i=0; i < count; i++)
    {
        ecs_meta_type_op_t *op = &ops[i];

        switch(op->kind)
        {
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
                if(op->name && op != ops) lua_setfield(L, -2, op->name);
                break;
            }
        }
    }

    lua_pop(L, 1);
}

static void deserialize_type(lua_State *L, int idx, ecs_meta_cursor_t *c, int depth)
{
    int ktype, vtype, ret, mtype, prev_ikey;
    bool in_array = false, designated_initializer = false;

    idx = lua_absindex(L, idx);

    vtype = lua_type(L, idx);
    mtype = 0;
    prev_ikey = 0;

    if(vtype == LUA_TTABLE)
    {
        ret = ecs_meta_push(c);
        ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);

        in_array = ecs_meta_is_collection(c);

        lua_pushnil(L);
    }
    else luaL_checktype(L, idx, LUA_TTABLE); // Error out (only struct/array components supported for now)

    while(lua_next(L, idx))
    {
        ktype = lua_type(L, -2);
        vtype = lua_type(L, -1);

        if(!mtype) mtype = ktype;

        if(ktype == LUA_TSTRING)
        {
            const char *key = lua_tostring(L, -2);

            ecs_lua_dbg_pad(depth);
            ecs_lua_dbg("meta_member field: %s", key);

            ret = ecs_meta_member(c, key);

            if(ret) luaL_error(L, "field \"%s\" does not exist", key);
            if(mtype != ktype) luaL_error(L, "table has mixed key types (string key '%s')", key);
        }
        else if(ktype == LUA_TNUMBER)
        {
            lua_Integer key = lua_tointeger(L, -2) - 1;

            ecs_lua_dbg_pad(depth);
            ecs_lua_dbg("move idx: %lld", key);

            //prefer not to trigger error messages in flecs
            //XXX: double check we're never entering designated init mode after a string key was found
            if(in_array)
            {
                ret = ecs_meta_elem(c, key);
            }
            else if(designated_initializer)
            {
                if(key == (prev_ikey + 1))
                {
                    ret = ecs_meta_next(c);
                    prev_ikey = key;
                }
                else ret = 1;
            }
            else if(!key)
            {
                designated_initializer = true;
            }
            else //designated initializer does not start with 0
            {
                ret = 1;
            }

            if(ret) luaL_error(L, "invalid index %I (Lua [%I])", key, key + 1);
            if(mtype != ktype) luaL_error(L, "table has mixed key types (int key [%I]", key+1); //XXX: move this up?
        }
        else luaL_error(L, "invalid key type '%s'", lua_typename(L, ktype));

        switch(vtype)
        {
            case LUA_TTABLE:
            {
                ecs_lua_dbg_pad(depth);
                ecs_lua_dbg("meta_push (nested)");
                //ecs_meta_push(c);
                //lua_pushnil(L);
                depth++;
                int top = lua_gettop(L);
                deserialize_type(L, top, c, depth);
                depth--;
                break;
            }
            case LUA_TNUMBER:
            {
                if(lua_isinteger(L, -1))
                {
                    lua_Integer integer = lua_tointeger(L, -1);

                    ecs_lua_dbg_pad(depth);
                    ecs_lua_dbg("set_int: %lld", integer);

                    /* XXX: Workaround for U64 bounds check */
                    ecs_meta_scope_t *scope = &c->scope[c->depth];
                    ecs_meta_type_op_t *op = &scope->ops[scope->op_cur];

                    if(op->kind == EcsOpU64) ret = ecs_meta_set_uint(c, integer);
                    else ret = ecs_meta_set_int(c, integer);

                    if(ret) luaL_error(L, "failed to set integer (%I)", integer);
                }
                else
                {
                    lua_Number number = lua_tonumber(L, -1);

                    ecs_lua_dbg_pad(depth);
                    ecs_lua_dbg("set_float %f", number);
                    ret = ecs_meta_set_float(c, number);

                    if(ret) luaL_error(L, "failed to set float (%f)", number);
                }

                break;
            }
            case LUA_TBOOLEAN:
            {
                ecs_lua_dbg_pad(depth);
                ecs_lua_dbg("set_bool: %d", lua_toboolean(L, -1));
                ret = ecs_meta_set_bool(c, lua_toboolean(L, -1));

                ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
                break;
            }
            case LUA_TSTRING:
            {
                ecs_lua_dbg_pad(depth);
                ecs_lua_dbg("set_string: %s", lua_tostring(L, -1));
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

    ecs_lua_dbg_pad(depth);
    ecs_lua_dbg("meta_pop");
    ret = ecs_meta_pop(c);
    ecs_assert(!ret, ECS_INTERNAL_ERROR, NULL);
}

static
void serialize_column(
    ecs_world_t *world,
    lua_State *L,
    const EcsMetaTypeSerialized *ser,
    const void *base,
    int32_t count)
{
    int32_t op_count = ecs_vec_count(&ser->ops);
    ecs_meta_type_op_t *ops = ecs_vec_first(&ser->ops);

    serialize_elements(world, ops, op_count, base, count, ops->size, L); //XXX: not sure about ops->size
}

static const EcsMetaTypeSerialized *get_serializer(lua_State *L, const ecs_world_t *world, ecs_entity_t type)
{
    return ecs_get(world, type, EcsMetaTypeSerialized);
//TODO: check if this optimization is still needed
    world = ecs_get_world(world);

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

        *ref = ecs_ref_init_id(world, type, ecs_id(EcsMetaTypeSerialized));

        lua_pop(L, 2); /* -types, -world */
    }

    const EcsMetaTypeSerialized *ser = ecs_ref_get_id(world, ref, 0);
    ecs_assert(ser != NULL, ECS_INTERNAL_ERROR, NULL);

    return ser;
}

static int columns__len(lua_State *L)
{
    ecs_iter_t *it = lua_touserdata(L, lua_upvalueindex(1));

    lua_pushinteger(L, it->field_count);

    return 1;
}

static int columns__index(lua_State *L)
{
    ecs_iter_t *it = lua_touserdata(L, lua_upvalueindex(1));
    ecs_world_t *world = it->world;

    lua_Integer i = luaL_checkinteger(L, 2);

    if(i < 1 || i > it->field_count) luaL_argerror(L, 1, "invalid term index");

    if(!it->count)
    {
        lua_pushnil(L);
        return 1;
    }

    ecs_entity_t type = ecs_get_typeid(world, ecs_field_id(it, i));
    const EcsMetaTypeSerialized *ser = get_serializer(L, world, type);

    if(!ser) luaL_error(L, "term %d cannot be serialized", i);

    lua_settop(L, 1); /* (it.)columns */

    const void *base = ecs_field_w_size(it, 0, i);

    if(!ecs_field_is_self(it, i)) serialize_type(world, &ser->ops, base, L);
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
    lua_createtable(L, it->field_count, 1);

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
    lua_pushinteger(L, it->count);
    lua_setfield(L, -2, "count");

    lua_pushinteger(L, it->system);
    lua_setfield(L, -2, "system");

    lua_pushinteger(L, it->event);
    lua_setfield(L, -2, "event");

    lua_pushinteger(L, it->event_id);
    lua_setfield(L, -2, "event_id");

//    lua_pushinteger(L, it->self);
//    lua_setfield(L, -2, "self");

    lua_pushnumber(L, it->delta_time);
    lua_setfield(L, -2, "delta_time");

    lua_pushnumber(L, it->delta_system_time);
    lua_setfield(L, -2, "delta_system_time");

    lua_pushinteger(L, it->table_count);
    lua_setfield(L, -2, "table_count");

    lua_pushinteger(L, it->term_index);
    lua_setfield(L, -2, "term_index");

    if(it->system)
    {
        ecs_lua_callback *sys = it->binding_ctx;

        if(sys->param_ref >= 0)
        {
            int type = ecs_lua_rawgeti(L, it->world, sys->param_ref);
            ecs_assert(type != LUA_TNIL, ECS_INTERNAL_ERROR, NULL);
        }
        else lua_pushnil(L);

        //lua_pushvalue(L, -1);
        //lua_setfield(L, -3, "ctx");

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
    ecs_assert(cursor->valid, ECS_INVALID_PARAMETER, NULL);
    ecs_assert(base != NULL, ECS_INVALID_PARAMETER, NULL);

    cursor->depth = 0;
    cursor->scope[0].op_cur = 0;
    cursor->scope[0].elem_cur = 0;
    cursor->scope[0].ptr = base;
}

static ecs_meta_cursor_t *ecs_lua_cursor(lua_State *L, const ecs_world_t *world, ecs_entity_t type, void *base)
{
    const ecs_world_t *real_world = ecs_get_world(world);

    int ret = lua_rawgetp(L, LUA_REGISTRYINDEX, real_world);
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

        *cursor = ecs_meta_cursor(world, type, base);
    }

    return cursor;
}

static
void deserialize_column(
    const ecs_world_t *world,
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
        deserialize_type(L, -1, c, 0);

        lua_pop(L, 1);
    }
}

void ecs_ptr_to_lua(
    const ecs_world_t *world,
    lua_State *L,
    ecs_entity_t type,
    const void *ptr)
{
    const EcsMetaTypeSerialized *ser = get_serializer(L, world, type);
    serialize_type(world, &ser->ops, ptr, L);
}

void ecs_lua_to_ptr(
    const ecs_world_t *world,
    lua_State *L,
    int idx,
    ecs_entity_t type,
    void *ptr)
{
    ecs_meta_cursor_t *c = ecs_lua_cursor(L, world, type, ptr);

    deserialize_type(L, idx, c, 0);
}

void ecs_lua_type_update(
    const ecs_world_t *world,
    lua_State *L,
    int idx,
    ecs_entity_t type,
    void *ptr)
{
    const EcsMetaTypeSerialized *ser = get_serializer(L, world, type);

    update_type(world, &ser->ops, ptr, L, idx);
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
    ecs_lua_dbg("ECS_LUA_TO_ITER");
    ecs_lua__prolog(L);
    ecs_iter_t *it = ecs_lua__checkiter(L, idx);
    ecs_world_t *world = it->world;
    const ecs_world_t *real_world = ecs_get_world(world);

    if(lua_getfield(L, idx, "interrupted_by") == LUA_TNUMBER) it->interrupted_by = lua_tointeger(L, -1);

    lua_pop(L, 1);

    /* newly-returned iterators have it->count = 0 */
    if(!it->count) return it;

    luaL_getsubtable(L, idx, "columns");
    luaL_checktype(L, -1, LUA_TTABLE);

    int32_t i;
    for(i=1; i <= it->field_count; i++)
    {
        if(it->next == ecs_query_next && ecs_field_is_readonly(it, i)) continue;

        int type = lua_rawgeti(L, -1, i); /* columns[i] */
        bool is_owned = ecs_field_is_self(it, i);

        if(type == LUA_TNIL)
        {
            ecs_lua_dbg("skipping empty term %d (not serialized?)", i);
            lua_pop(L, 1);
            continue;
        }

        if(is_owned) { ecs_assert(it->count == lua_rawlen(L, -1), ECS_INTERNAL_ERROR, NULL); }

        int32_t count = it->count;
        ecs_entity_t column_entity = ecs_get_typeid(world, ecs_field_id(it, i));
        void *base = ecs_field_w_size(it, 0, i);

        if(!is_owned) ecs_lua_to_ptr(world, L, -1, column_entity, base);
        else deserialize_column(world, L, -1, column_entity, base, ecs_field_size(it, i), count);

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
bool ecs_lua_iter_next(lua_State *L, int idx)
{ecs_lua_dbg("ITER_NEXT");
    ecs_iter_t *it = ecs_lua_to_iter(L, idx);

    if(!ecs_iter_next(it)) return false;

    ecs_lua_iter_update(L, idx, it);

    return true;
}

int meta_constants(lua_State *L)
{
    ecs_world_t *w = ecs_lua_world(L);

    ecs_entity_t type = luaL_checkinteger(L, 1);
    const char *prefix = luaL_optstring(L, 3, NULL);
    const char *flags = luaL_optstring(L, 4, "");

    int lowercase = strchr(flags, 'l') ? 1 : 0;

    const EcsMetaType *meta = ecs_get(w, type, EcsMetaType);

    if(!meta) luaL_argerror(L, 1, "invalid type");
    if(meta->kind != EcsEnumType && meta->kind != EcsBitmaskType) luaL_argerror(L, 1, "not an enum/bitmask");

    const EcsMetaTypeSerialized *ser = get_serializer(L, w, type);
    ecs_meta_type_op_t *op = ecs_vec_first(&ser->ops);

    if(lua_type(L, 2) == LUA_TTABLE) lua_pushvalue(L, 2);
    else lua_newtable(L);

    serialize_constants(w, op, L, prefix, lowercase);

    return 1;
}

static void each_reset_columns(lua_State *L, ecs_lua_each_t *each)
{
    ecs_iter_t *it = each->it;
    ecs_lua_col_t *col = each->cols;
    const ecs_world_t *world = it->world;

    each->i = 0;

    int i;
    for(i=1; i <= it->field_count; i++, col++)
    {
        ecs_id_t field_id = ecs_field_id(it, i);
        ecs_assert(field_id != 0, ECS_INTERNAL_ERROR, NULL);

        col->type = ecs_get_typeid(world, field_id);
        col->stride = ecs_field_size(it, i);
        col->ptr = ecs_field_w_size(it, 0, i);
        col->ser = get_serializer(L, world, col->type);
        col->cursor = ecs_lua_cursor(L, it->world, col->type, col->ptr);

        if(!ecs_field_is_self(it, i)) col->stride = 0;

        if(it->next == ecs_query_next && ecs_field_is_readonly(it, i)) col->readback = false;
        else col->readback = true;

        col->update = true;
    }
}

static int empty_next_func(lua_State *L)
{
    return 0;
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

    for(j=0; j < it->field_count; j++, col++)
    {
        if(!col->readback) continue;

        ecs_lua_dbg("each() readback: %d", i-1);

        idx = lua_upvalueindex(j+2);
        ptr = ECS_OFFSET(col->ptr, col->stride * (i - 1));

        meta_reset(col->cursor, ptr);
        deserialize_type(L, idx, col->cursor, 0);
    }

    col = each->cols;

skip_readback:

    each->read_prev = true;

    if(i == it->count)
    {
        if(each->from_query)
        {
            if(ecs_lua_iter_next(L, 1)) each_reset_columns(L, each);
            else end = true;
        }
        else end = true;
    }

    if(end) return 0;

    for(j=0; j < it->field_count; j++, col++)
    {// optimization: shared fields should be read back at the end
        if(!col->update) continue;

        idx = lua_upvalueindex(j+2);
        ptr = ECS_OFFSET(col->ptr, col->stride * i);

        lua_pushvalue(L, idx);
        update_type(each->it->real_world, &col->ser->ops, ptr, L, idx);
    }

    lua_pushinteger(L, it->entities[i]);

    each->i++;

    return it->field_count + 1;
}

static int is_primitive(const EcsMetaTypeSerialized *ser)
{
    ecs_assert(ser != NULL, ECS_INVALID_PARAMETER, NULL);

    ecs_meta_type_op_t *ops = (ecs_meta_type_op_t*)ecs_vec_first(&ser->ops);
    int32_t count = ecs_vec_count(&ser->ops);

    if(count != 2) return 0;

    if(ops[1].kind = EcsOpPrimitive) return 1;

    return 0;
}

int each_func(lua_State *L)
{ecs_lua_dbg("ecs.each()");
    ecs_world_t *w = ecs_lua_world(L);
    ecs_query_t *q = NULL;
    ecs_iter_t *it;
    int iter_idx = 1;

    if(lua_type(L, 1) == LUA_TUSERDATA)
    {
        q = checkquery(L, 1);
        ecs_iter_t iter = ecs_query_iter(w, q);
        int b = ecs_query_next(&iter); // XXX: will next_func iterate and skip data?

        if(!b) /* no matching entities */
        {/* must return an iterator, push one that ends it immediately */
            lua_pushcfunction(L, empty_next_func);
            lua_pushinteger(L, 0);
            lua_pushinteger(L, 1);
            return 3;
        }

        it = ecs_iter_to_lua(&iter, L, true);
        iter_idx = lua_gettop(L);
    }
    else it = ecs_lua__checkiter(L, 1);

    size_t size = sizeof(ecs_lua_each_t) + it->field_count * sizeof(ecs_lua_col_t);
    ecs_lua_each_t *each = lua_newuserdata(L, size);

    each->it = it;
    each->from_query = q ? true : false;
    each->read_prev = false;

    each_reset_columns(L, each);

    int i;
    for(i=1; i <= it->field_count; i++)
    {
        lua_newtable(L);

        const EcsMetaTypeSerialized *ser = each->cols[i].ser;
        //if(!ser) luaL_error(L, "col");
        //if(is_primitive(ser)) luaL_error(L, "primitive");
    }

    lua_pushcclosure(L, next_func, it->field_count + 1);

    /* it */
    lua_pushvalue(L, iter_idx);

    lua_pushinteger(L, 1);

    return 3;
}
