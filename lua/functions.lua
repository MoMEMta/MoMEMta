-- Make an object immutable, preventing the user to modify it
local function const(obj)
    local mt = {}
    function mt.__index(proxy, k)
        local v = obj[k]
        if type(v) == 'table' then
            v = const(v)
        end
        return v
    end

    function mt.__newindex(proxy, k, v)
        error("object is constant", 2)
    end

    -- Allow modification of the original object if you know what you are doing
    mt.__obj = obj

    local tc = setmetatable({}, mt)
    return tc
end

-- merge two tables. t1 is modified
function append(t1, t2)
    for i = 1, #t2 do
        t1[#t1 + 1] = t2[i]
    end

    return t1
end

-- merge two tables. t1 and t2 are not modified
function copy_and_append(t1, t2)
    local t3 = {}

    append(t3, t1)
    append(t3, t2)

    return t3
end

-- Declare a new input:
-- Arguments:
--   - name: name of the input
--
-- An input exposes the following attributes:
--   - reco_p4: InputTag for the reco lorentz vector, ie the lorentz vector passed by the user
--   - gen_p4: InputTag for the gen lorentz vector. By default, it's the same as `reco_p4`, but it can be changed
--             by calling `set_gen_p4`
--   - name: The name of the input (user defined)
--   - type: The type of the input
function declare_input(name)

    if type(name) ~= "string" then
        error("input name must be a string")
    end

    local input = {
        name = name,
        reco_p4 = name .. "::p4",
        type = name .. "::type"
    }

    input.gen_p4 = input.reco_p4
    input.set_gen_p4 = function(tag)
        input.gen_p4 = tag
    end

    -- C function defined by MoMEMta
    momemta_declare_input(name)

    return const(input)
end

-- Enable permutations on reco particles
function add_reco_permutations(...)
    -- Function arguments
    local args = {...}
    local nargs = #args

    local module_name = {"permutate_reco_"}
    local inputs = {}
    for index, input in pairs(args) do
        append(module_name, {input.name})
        if index ~= nargs then
            append(module_name, {"_and_"})
        end

        append(inputs, {input.reco_p4})
    end

    module_name = table.concat(module_name)

    if Permutator[module_name] ~= nil then
        error("A permutator named " .. module_name .. " already exists")
    end

    Permutator[module_name] = {
        ps_point = add_dimension(),
        inputs = inputs
    }

    -- Redefine reco & gen p4 input tags to point to the permutator output
    for i = 1, nargs do
        getmetatable(args[i]).__obj.reco_p4 = module_name .. "::output/" .. i
        args[i].set_gen_p4(args[i].reco_p4)
    end
end

-- Enable permutations on gen particles
function add_gen_permutations(...)
    -- Function arguments
    local args = {...}
    local nargs = #args

    local module_name = {"permutate_gen_"}
    local inputs = {}
    for index, input in pairs(args) do
        append(module_name, {input.name})
        if index ~= nargs then
            append(module_name, {"_and_"})
        end

        append(inputs, {input.gen_p4})
    end

    module_name = table.concat(module_name)

    if Permutator[module_name] ~= nil then
        error("A permutator named " .. module_name .. " already exists")
    end

    Permutator[module_name] = {
        ps_point = add_dimension(),
        inputs = inputs
    }

    -- Redefine gen p4 input tag to point to the permutator output
    for i = 1, nargs do
        args[i].set_gen_p4(module_name .. "::output/" .. i)
    end
end