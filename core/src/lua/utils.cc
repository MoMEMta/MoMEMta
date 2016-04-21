#include <stdexcept>

#include <momemta/InputTag.h>
#include <momemta/ConfigurationSet.h>
#include <momemta/IOnModuleDeclared.h>
#include <momemta/ModuleFactory.h>
#include <momemta/Utils.h>

#include <lua/utils.h>
#include <LibraryManager.h>

namespace lua {

    Lazy::Lazy(lua_State* L) {
        this->L = L;
    }

    LazyFunction::LazyFunction(lua_State* L, int index): Lazy(L) {
        auto absolute_index = get_index(L, index);

        // Duplicate the function on the top of the stack. This ensure the stack size won't change
        lua_pushvalue(L, absolute_index);

        // Pop the anonymous function from the stack, and store it in the global lua registry
        ref_index = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    boost::any LazyFunction::operator() () const {

        TRACE("[LazyFunction::operator()] >> stack size = {}", lua_gettop(L));

        // Pop the anonymous function from the registry, and push it on the top of the stack
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref_index);

        // Call the function. The function removed from the stack, and the return value is pushed on the top of the stack
        auto result = lua_pcall(L, 0, 1, 0);
        if (result != LUA_OK) {
            std::string error = lua_tostring(L, -1);
            LOGGER->critical("Fail to call lua anonymous function. Return value is {}. Error message: {}", result, error);
        }

        boost::any value;
        bool lazy = false;
        std::tie(value, lazy) = to_any(L, -1);
        assert(!lazy);

        lua_pop(L, 1);

        TRACE("[LazyFunction::operator()] << stack size = {}", lua_gettop(L));

        return value;
    }

    LazyTableField::LazyTableField(lua_State* L, const std::string& table_name, const std::string& key):
        Lazy(L) {
        this->table_name = table_name;
        this->key = key;
    }

    boost::any LazyTableField::operator() () const {
        TRACE("[LazyTableField::operator()] >> stack size = {}", lua_gettop(L));

        // Push the table on the stack. Stack +1
        lua_getglobal(L, table_name.c_str());

        // Push the requested field from the table to the stack. Stack +1
        lua_getfield(L, -1, key.c_str());

        boost::any value;
        bool lazy = false;
        std::tie(value, lazy) = to_any(L, -1);
        assert(!lazy);

        // Pop the field and the table from the stack. Stack -2
        lua_pop(L, 2);

        TRACE("[LazyTableField::operator()] << stack size = {}", lua_gettop(L));

        return value;
    }

    void LazyTableField::set(const boost::any& value) {
        TRACE("[LazyTableField::set] >> stack size = {}", lua_gettop(L));

        // Push the table on the stack. Stack +1
        lua_getglobal(L, table_name.c_str());

        // Push the value to the stack. Stack +1
        lua::push_any(L, value);

        // Pop the requested field from the stack and assign it to the table. Stack -1
        lua_setfield(L, -2, key.c_str());

        // Pop the table from the stack. Stack -1
        lua_pop(L, 1);

        TRACE("[LazyTableField::set] << stack size = {}", lua_gettop(L));
    }

    Type type(lua_State* L, int index) {
        int t = lua_type(L, index);

        switch (t) {
            case LUA_TBOOLEAN:
                return BOOLEAN;
                break;

            case LUA_TSTRING: {
                std::string value = lua_tostring(L, index);
                if (InputTag::isInputTag(value))
                    return INPUT_TAG;
                else
                    return STRING;
            } break;

            case LUA_TNUMBER:
                if (lua_isinteger(L, index))
                    return INTEGER;
                else
                    return REAL;
                break;

            case LUA_TTABLE: {
                // We only support ConfigurationSet for table
                if (lua_is_array(L, index) == -1) {
                    return CONFIGURATION_SET;
                }

            } break;
        }

        return NOT_SUPPORTED;
    }

    size_t get_index(lua_State* L, int index) {
        return (index < 0) ? lua_gettop(L) + index + 1 : index;
    }

    /*!
     *  Check if a lua table is an array
     *
     *  Return -1 if it's not an array, or the size of the array
     */
    int lua_is_array(lua_State* L, int index) {

        TRACE("[lua_is_array] >> stack size = {}", lua_gettop(L));

        size_t table_index = get_index(L, index);

        if (lua_type(L, table_index) != LUA_TTABLE)
            return -1;

        size_t size = 0;

        lua_pushnil(L);
        while (lua_next(L, table_index) != 0) {
            if (lua_type(L, -2) != LUA_TNUMBER) {
                lua_pop(L, 2);
                TRACE("[lua_is_array] << stack size = {}", lua_gettop(L));
                return -1;
            }

            size++;

            lua_pop(L, 1);
        }

        TRACE("[lua_is_array] << stack size = {}", lua_gettop(L));
        return size;
    }

    /*!
     *  \brief Check if a lua table contains only value from the same type
     *
     *  \return -1 if not, or the type of the data in the table
     */
    Type lua_array_unique_type(lua_State* L, int index) {

        size_t absolute_index = get_index(L, index);

        if (lua_type(L, absolute_index) != LUA_TTABLE)
            return NOT_SUPPORTED;

        Type result = NOT_SUPPORTED;

        lua_pushnil(L);
        while (lua_next(L, absolute_index) != 0) {
            if (result == NOT_SUPPORTED) {
                result = type(L, -1);
            } else {
                Type entry_type = type(L, -1);

                if ((result == INTEGER) && (entry_type == REAL))
                    result = REAL;
                else if ((result == REAL) && (entry_type == INTEGER))
                    result = REAL;
                else if (result != entry_type) {
                    lua_pop(L, 2);
                    result = NOT_SUPPORTED;
                    break;
                }
            }

            lua_pop(L, 1);
        }

        return result;
    }

    std::pair<boost::any, bool> to_any(lua_State* L, int index) {

        TRACE("[to_any] >> stack size = {}", lua_gettop(L));
        size_t absolute_index = get_index(L, index);

        boost::any result;
        bool lazy = false;

        auto type = lua_type(L, absolute_index);
        switch (type) {
            case LUA_TNUMBER: {
                if (lua_isinteger(L, absolute_index)) {
                    int64_t number = lua_tointeger(L, absolute_index);
                    result = number;
                } else {
                    double number = lua_tonumber(L, absolute_index);
                    result = number;
                }
            } break;

            case LUA_TBOOLEAN: {
                bool value = lua_toboolean(L, absolute_index);
                result = value;
            } break;

            case LUA_TSTRING: {
                std::string value = lua_tostring(L, absolute_index);
                if (InputTag::isInputTag(value)) {
                    InputTag tag = InputTag::fromString(value);
                    result = tag;
                } else {
                    result = value;
                }
            } break;

            case LUA_TTABLE: {
                TRACE("[to_any::table] >> stack size = {}", lua_gettop(L));
                if (lua::lua_is_array(L, absolute_index) > 0) {

                    Type type = NOT_SUPPORTED;

                    if ((type = lua::lua_array_unique_type(L, absolute_index)) != NOT_SUPPORTED) {
                        result = to_vector(L, absolute_index, type);
                    } else {
                        throw invalid_array_error("Various types stored into the array. This is not supported.");
                    }

                } else {
                    ConfigurationSet cfg;
                    cfg.parse(L, absolute_index);
                    result = cfg;
                }
                TRACE("[to_any::table] << stack size = {}", lua_gettop(L));
            } break;

            case LUA_TFUNCTION: {
                TRACE("[to_any::function] >> stack size = {}", lua_gettop(L));

                result = LazyFunction(L, absolute_index);
                lazy = true;

                TRACE("[to_any::function] << stack size = {}", lua_gettop(L));
            } break;

            default: {
                LOGGER->critical("Unsupported lua type: {}", lua_typename(L, type));
                throw lua::invalid_configuration_file("");
            } break;
        }

        TRACE("[to_any] << final type = {}", demangle(result.type().name()));
        TRACE("[to_any] << stack size = {}", lua_gettop(L));
        return {result, lazy};
    }

    void push_any(lua_State* L, const boost::any& value) {
        TRACE("[push_any] >> stack size = {}", lua_gettop(L));

        if (value.type() == typeid(int64_t)) {
            int64_t v = boost::any_cast<int64_t>(value);
            lua_pushinteger(L, v);
        } else if (value.type() == typeid(double)) {
            double v = boost::any_cast<double>(value);
            lua_pushnumber(L, v);
        } else if (value.type() == typeid(bool)) {
            bool v = boost::any_cast<bool>(value);
            lua_pushboolean(L, v);
        } else if (value.type() == typeid(std::string)) {
            auto v = boost::any_cast<std::string>(value);
            lua_pushstring(L, v.c_str());
        } else {
            LOGGER->critical("Unsupported C++ value: {}", demangle(value.type().name()));
            throw lua::unsupported_type_error(demangle(value.type().name()));
        }

        TRACE("[push_any] << stack size = {}", lua_gettop(L));
    }

    boost::any to_vector(lua_State* L, int index, Type t) {
        switch (t) {
            case BOOLEAN:
                return to_vectorT<bool>(L, index);

            case STRING:
                return to_vectorT<std::string>(L, index);

            case INTEGER:
                return to_vectorT<int64_t>(L, index);

            case REAL:
                return to_vectorT<double>(L, index);

            case INPUT_TAG:
                return to_vectorT<InputTag>(L, index);

            case CONFIGURATION_SET:
                return to_vectorT<ConfigurationSet>(L, index);

            case NOT_SUPPORTED:
                break;
        }

        throw invalid_array_error("Unsupported array type");
    }

    //! Specialization for double type, with implicit conversion from integer
    template<> double special_any_cast(const boost::any& value) {
        if (value.type() == typeid(int64_t))
            return static_cast<double>(boost::any_cast<int64_t>(value));

        return boost::any_cast<double>(value);
    }

    int module_table_newindex(lua_State* L) {

        lua_getmetatable(L, 1);
        lua_getfield(L, -1, "__type");

        const char* module_type = luaL_checkstring(L, -1);
        const char* module_name = luaL_checkstring(L, 2);

        // Remove field name from stack
        lua_pop(L, 1);

        lua_getfield(L, -1, "__ptr");
        void* cfg_ptr = lua_touserdata(L, -1);
        IOnModuleDeclared* callback = static_cast<IOnModuleDeclared*>(cfg_ptr);

        callback->onModuleDeclared(module_type, module_name);

        // Remove metatable and field name from stack
        lua_pop(L, 2);

        // And actually set the value to the table
        lua_rawset(L, 1);

        return 0;
    }

    void register_modules(lua_State* L, void* ptr) {
        std::vector<std::string> modules = ModuleFactory::get().getPluginsList();
        for (const auto& module: modules) {
            const char* module_name = module.c_str();
            char* module_metatable = new char[module.size() + 3 + 1];
            strncpy(module_metatable, module_name, module.size() + 1);
            strncat(module_metatable, "_mt", 3);

            int type = lua_getglobal(L, module_name);
            lua_pop(L, 1);
            if (type != LUA_TNIL) {
                // Global already exists
                continue;
            }

            // Create a new empty table
            lua_newtable(L);

            // Create the associated metatable
            luaL_newmetatable(L, module_metatable);

            lua_pushstring(L, module_name);
            lua_setfield(L, -2, "__type");

            lua_pushlightuserdata(L, ptr);
            lua_setfield(L, -2, "__ptr");

            // Set the metadata '__newindex' function
            const luaL_Reg l[] = {
                {"__newindex", lua::module_table_newindex},
                {nullptr, nullptr}
            };
            luaL_setfuncs(L, l, 0);

            lua_setmetatable(L, -2);

            // And register it as a global variable
            lua_setglobal(L, module_name);

            TRACE("Registered new lua global variable '{}'", module_name);

            delete[] module_metatable;
        }
    }

    int load_modules(lua_State* L) {
        int n = lua_gettop(L);
        if (n != 1) {
            luaL_error(L, "invalid number of arguments: 1 expected, got %d", n);
        }

        void* cfg_ptr = lua_touserdata(L, lua_upvalueindex(1));

        const char *path = luaL_checkstring(L, 1);
        LibraryManager::get().registerLibrary(path);

        register_modules(L, cfg_ptr);

        return 0;
    }

    int parameter(lua_State* L) {
        int n = lua_gettop(L);
        if (n != 1) {
            luaL_error(L, "invalid number of arguments: 1 expected, got %d", n);
        }

        std::string parameter_name = luaL_checkstring(L, 1);

        // Create an anonymous function return the value of the parameter
        // Assumes there's a global table named `configuration`

        std::string code = "return function() return configuration['" + parameter_name + "'] end";
        luaL_dostring(L, code.c_str());

        return 1;
    }

    void setup_hooks(lua_State* L, void* ptr) {
        lua_pushlightuserdata(L, ptr);
        lua_pushcclosure(L, load_modules, 1);
        lua_setglobal(L, "load_modules");

        lua_pushlightuserdata(L, ptr);
        lua_pushcclosure(L, parameter, 1);
        lua_setglobal(L, "parameter");
    }

    std::shared_ptr<lua_State> init_runtime(IOnModuleDeclared* callback) {

        std::shared_ptr<lua_State> L(luaL_newstate(), lua_close);
        luaL_openlibs(L.get());

        // Register function load_modules
        lua::setup_hooks(L.get(), callback);

        // Register existing modules
        lua::register_modules(L.get(), callback);

        return L;

    }
}
