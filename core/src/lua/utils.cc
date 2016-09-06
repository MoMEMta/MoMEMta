/*
 *  MoMEMta: a modular implementation of the Matrix Element Method
 *  Copyright (C) 2016  Universite catholique de Louvain (UCL), Belgium
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <lua/utils.h>

#include <momemta/InputTag.h>
#include <momemta/ILuaCallback.h>
#include <momemta/Logging.h>
#include <momemta/ModuleFactory.h>
#include <momemta/ParameterSet.h>
#include <momemta/Utils.h>

#include <LibraryManager.h>
#include <lua/Path.h>
#include <lua/Types.h>

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

        LOG(trace) << "[LazyFunction::operator()] >> stack size = " << lua_gettop(L);

        // Pop the anonymous function from the registry, and push it on the top of the stack
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref_index);

        // Call the function. The function removed from the stack, and the return value is pushed on the top of the stack
        auto result = lua_pcall(L, 0, 1, 0);
        if (result != LUA_OK) {
            std::string error = lua_tostring(L, -1);
            LOG(fatal) << "Fail to call lua anonymous function. Return value is " << result << ". Error message: " << error;
        }

        boost::any value;
        bool lazy = false;
        std::tie(value, lazy) = to_any(L, -1);
        assert(!lazy);

        lua_pop(L, 1);

        LOG(trace) << "[LazyFunction::operator()] << stack size = " << lua_gettop(L);

        return value;
    }

    LazyTableField::LazyTableField(lua_State* L, const std::string& table_name, const std::string& key):
        Lazy(L) {
        this->table_name = table_name;
        this->key = key;
    }

    void LazyTableField::ensure_created() {
        // Push the table on the stack. Stack +1
        int type = lua_getglobal(L, table_name.c_str());
        // Discard the result. Stack -1
        lua_pop(L, 1);

        // Already existing
        if (type != LUA_TNIL)
            return;

        // Create a new table. Stack +1
        lua_newtable(L);
        // Set it global. Stack -1
        lua_setglobal(L, table_name.c_str());
    }

    boost::any LazyTableField::operator() () const {
        LOG(trace) << "[LazyTableField::operator()] >> stack size = " << lua_gettop(L);

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

        LOG(trace) << "[LazyTableField::operator()] << stack size = " << lua_gettop(L);

        return value;
    }

    void LazyTableField::set(const boost::any& value) {
        LOG(trace) << "[LazyTableField::set] >> stack size = " << lua_gettop(L);

        // Push the table on the stack. Stack +1
        lua_getglobal(L, table_name.c_str());

        // Push the value to the stack. Stack +1
        lua::push_any(L, value);

        // Pop the requested field from the stack and assign it to the table. Stack -1
        lua_setfield(L, -2, key.c_str());

        // Pop the table from the stack. Stack -1
        lua_pop(L, 1);

        LOG(trace) << "[LazyTableField::set] << stack size = " << lua_gettop(L);
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
                // We only support ParameterSet for table
                if (lua_is_array(L, index) == -1) {
                    return PARAMETER_SET;
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

        LOG(trace) << "[lua_is_array] >> stack size = " << lua_gettop(L);

        size_t table_index = get_index(L, index);

        if (lua_type(L, table_index) != LUA_TTABLE)
            return -1;

        size_t size = 0;

        lua_pushnil(L);
        while (lua_next(L, table_index) != 0) {
            if (lua_type(L, -2) != LUA_TNUMBER) {
                lua_pop(L, 2);
                LOG(trace) << "[lua_is_array] << stack size = " << lua_gettop(L);
                return -1;
            }

            size++;

            lua_pop(L, 1);
        }

        LOG(trace) << "[lua_is_array] << stack size = " << lua_gettop(L);
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

        LOG(trace) << "[to_any] >> stack size = " << lua_gettop(L);
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
                LOG(trace) << "[to_any::table] >> stack size = " << lua_gettop(L);
                if (lua::lua_is_array(L, absolute_index) > 0) {

                    Type type = NOT_SUPPORTED;

                    if ((type = lua::lua_array_unique_type(L, absolute_index)) != NOT_SUPPORTED) {
                        result = to_vector(L, absolute_index, type);
                    } else {
                        throw invalid_array_error("Various types stored into the array. This is not supported.");
                    }

                } else {
                    ParameterSet cfg;
                    cfg.parse(L, absolute_index);
                    result = cfg;
                }
                LOG(trace) << "[to_any::table] << stack size = " << lua_gettop(L);
            } break;

            case LUA_TFUNCTION: {
                LOG(trace) << "[to_any::function] >> stack size = " << lua_gettop(L);

                result = LazyFunction(L, absolute_index);
                lazy = true;

                LOG(trace) << "[to_any::function] << stack size = " << lua_gettop(L);
            } break;

            case LUA_TUSERDATA: {
                result = get_custom_type_ptr(L, absolute_index);

            } break;

            default: {
                LOG(fatal) << "Unsupported lua type: " << lua_type(L, absolute_index);
                throw lua::invalid_configuration_file("");
            } break;
        }

        LOG(trace) << "[to_any] << final type = " << demangle(result.type().name());
        LOG(trace) << "[to_any] << stack size = " << lua_gettop(L);
        return {result, lazy};
    }

    void push_any(lua_State* L, const boost::any& value) {
        LOG(trace) << "[push_any] >> stack size = " << lua_gettop(L);

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
        } else if (value.type() == typeid(InputTag)) {
            auto v = boost::any_cast<InputTag>(value).toString();
            lua_pushstring(L, v.c_str());
        } else {
            LOG(fatal) << "Unsupported C++ value: " << demangle(value.type().name());
            throw lua::unsupported_type_error(demangle(value.type().name()));
        }

        LOG(trace) << "[push_any] << stack size = " << lua_gettop(L);
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

            case PARAMETER_SET:
                return to_vectorT<ParameterSet>(L, index);

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
        ILuaCallback* callback = static_cast<ILuaCallback*>(cfg_ptr);

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

            int type = lua_getglobal(L, module_name);
            lua_pop(L, 1);
            if (type != LUA_TNIL) {
                // Global already exists
                continue;
            }

            // Create a new empty table
            lua_newtable(L);

            std::string module_metatable = module + "_mt";

            // Create the associated metatable
            luaL_newmetatable(L, module_metatable.c_str());

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

            LOG(trace) << "Registered new lua global variable '" << module_name << "'";
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

        std::string code = "return function() return parameters['" + parameter_name + "'] end";
        luaL_dostring(L, code.c_str());

        return 1;
    }

    int set_final_module(lua_State* L) {
        int n = lua_gettop(L);
        if (n == 0) {
            luaL_error(L, "invalid number of arguments: at least one expected, got 0");
        }

        void* cfg_ptr = lua_touserdata(L, lua_upvalueindex(1));
        ILuaCallback* callback = static_cast<ILuaCallback*>(cfg_ptr);
        
        for(size_t i = 1; i <= size_t(n); i++) {
            std::string input_tag = luaL_checkstring(L, i);
            if (!InputTag::isInputTag(input_tag)) {
                luaL_error(L, "'%s' is not a valid InputTag", input_tag.c_str());
            }
            callback->onIntegrandDeclared(InputTag::fromString(input_tag));
        }

        return 0;
    }

    int add_integration_dimension(lua_State* L) {
        int n = lua_gettop(L);
        if (n != 0) {
            luaL_error(L, "invalid number of arguments: 0 expected, got %d", n);
        }

        // Create input tag using current value of the index
        int64_t cuba_index = lua_tonumber(L, lua_upvalueindex(1));
        lua_pushnumber(L, cuba_index + 1);
        lua_replace(L, lua_upvalueindex(1));

        std::string index_tag = "cuba::ps_points/";
        index_tag += std::to_string(cuba_index);

        // Input tag is return value of the function
        push_any(L, index_tag);
        
        // Add an integration dimension in the configuration
        void* cfg_ptr = lua_touserdata(L, lua_upvalueindex(2));
        ILuaCallback* callback = static_cast<ILuaCallback*>(cfg_ptr);
        callback->addIntegrationDimension();

        return 1;
    }

    void setup_hooks(lua_State* L, void* ptr) {
        lua_pushlightuserdata(L, ptr);
        lua_pushcclosure(L, load_modules, 1);
        lua_setglobal(L, "load_modules");

        lua_pushlightuserdata(L, ptr);
        lua_pushcclosure(L, parameter, 1);
        lua_setglobal(L, "parameter");

        // Define the `add_dimension()` function in Lua and make it available in the global namespace.
        // See add_integration_dimension for more information.
        lua_pushnumber(L, 1);
        lua_pushlightuserdata(L, ptr);
        lua_pushcclosure(L, add_integration_dimension, 2);
        lua_setglobal(L, "add_dimension");

        // integrand() function
        lua_pushlightuserdata(L, ptr);
        lua_pushcclosure(L, set_final_module, 1);
        lua_setglobal(L, "integrand");

        path_register(L, ptr);
    }

    std::shared_ptr<lua_State> init_runtime(ILuaCallback* callback) {

        std::shared_ptr<lua_State> L(luaL_newstate(), lua_close);
        luaL_openlibs(L.get());

        // Register hooks function, like `load_modules`
        lua::setup_hooks(L.get(), callback);

        // Register existing modules
        lua::register_modules(L.get(), callback);
    
        return L;
    }
}
