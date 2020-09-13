local ecs = require "ecs"

local m = {}

function string.starts(str, start)
    return str:sub(1, #start) == start
end

function m.print_entities(...)
    print("Entities:")
    local args = {...}
    for i,v in ipairs(args) do
        print("\tid: " .. v .. ", name: " .. ecs.name(v))
    end
end

function m.print_constants(...)
    print("Constants:")
    local args = {...}
    for i,v in ipairs(args) do
        print("\tecs." .. v, "= " .. ecs[v])
    end
end

function m.print_array(a)
    for i, v in ipairs(a) do
        print("[" .. i .. "] = " .. v)
    end
end

function m.print_packages()
    print("Loaded packages:")
    for k, v in pairs(package.loaded) do
        print("\tname: ", k)
    end
end

return m