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

function m.print_ecs_ints()
    print("ecs constants:")
    for k, v in pairs(ecs) do
        if type(v) == "number" then
            print(k .. " = " ..  v)
        end
    end
end

function m.default_progress()
    ecs.progress_cb(function() ecs.log("progress()!") end)
end

function m.test_defaults()
    m.default_progress()
    ecs.tracing_color_enable()
end

function m.asserteq(a, b)
    if(a ~= b) then
        a = a or "(nil)"
        b = b or "(nil)"
        error("assertion failed: " .. a .. " != " .. b, 2)
    end
end

function m.starts_with(str, start)
    return str:sub(1, #start) == start
end

function m.ends_with(str, ending)
    return ending == "" or str:sub(-#ending) == ending
end

return m