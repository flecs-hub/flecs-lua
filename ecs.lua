--EmmyLua annotations and documentation for flecs-lua
--NOTE: ecs is a native module, do not use this at runtime.

local ecs = {}

---Create a new entity, all parameters are optional
---@param id integer @optional
---@param name string @optional
---@param components string
---@return integer @entity
function ecs.new(id, name, components)
end

---Create N new entities with optional components
---@param type string @optional
---@param n integer
---@return integer[]
function ecs.bulk_new(type, n)
end

---Delete an entity and all of its components
---@param entity integer|string
function ecs.delete(entity)
end

---Create a tag
---@param name integer
---@return integer @entity
function ecs.tag(name)
end

---Get the name of an entity
---@param id integer
---@return string|nil @entity name
function ecs.name(id)
end

---Look up an entity by name
---@param name string
---@return string|nil @entity name
function ecs.lookup(name)
end

---Test if an entity has a component, type or tag
---@param id integer @entity
---@param type string|integer
---@return boolean
function ecs.has(id, type)
end

---Add a component, type or tag to an entity
---@param id integer entity
---@param type string|integer
function ecs.add(id, type)
end

---Remove a component, type or tag from an entity
---@param id integer @entity
---@param type string|integer
function ecs.remove(id, type)
end

---Clear all components
---@param id integer entity
function ecs.clear(id)
end

---Create a meta array component
---@param name string entity name
---@param description string @format: "(type,N)"
---@return integer @entity
function ecs.array(name, description)
end

---Create a meta struct component
---@param name string entity name
---@param description string @format: "{type member; ...}"
---@return integer @entity
function ecs.struct(name, description)
end

---Create an alias for a meta type
---@param meta_type string @name
---@param alias string
---@return integer @entity
function ecs.alias(meta_type, alias)
end

---Create a system - DRAFT
---@param func function
---@param name string
---@param phase integer
---@param signature string @optional
---@return integer @entity
function ecs.system(func, name, phase, signature)
end

---Create a module - DRAFT
---@param name string
---@return integer @entity
function ecs.module(name)
end

--[[
---Import a Lua module - DRAFT
---@param module string
---@return table
function ecs.import(module)
    --only C modules are loaded once?
    if(package.loaded[module]) then
        return package.loaded[module]
    end

    local m = require(module)
    m.import()
    return m
end
]]--
return ecs