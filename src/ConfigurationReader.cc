#include <boost/any.hpp>
#include <logging.h>
#include <lua.hpp>

#include <ConfigurationReader.h>
#include <lua/utils.h>
#include <LibraryManager.h>
#include <ModuleFactory.h>

namespace lua {

    /*
     * For each type of module existing, declare a global variable named like
     * the module's type. This variable is a table.
     *
     * Each entry in this table declare a new module. The key of the entry is the module's name, and the name of the table where the entry is declared is the module's type. The value is iteself a table, defining the configuration of the module
     *
     */

    /*!
     * Hook for module declaration
     *
     * The stack contains 3 elements
     *     1        the module type table
     *     2        the module name (the key in the table)
     *     3        the module definition (the value associated with the key)
     */
    int module_table_newindex(lua_State* L) {

        lua_getmetatable(L, 1);
        lua_getfield(L, -1, "__name");
        
        const char* module_type = luaL_checkstring(L, -1);
        const char* module_name = luaL_checkstring(L, 2);

        // Remove field name from stack
        lua_pop(L, 1);

        lua_getfield(L, -1, "__ptr");
        void* cfg_ptr = lua_touserdata(L, -1);
        ConfigurationReader* reader = static_cast<ConfigurationReader*>(cfg_ptr);

        reader->addModule(module_type, module_name);
        
        // Remove metatable and field name from stack
        lua_pop(L, 2);

        lua_rawset(L, 1);

        return 0;
    }

    void lua_register_modules(lua_State* L, void* ptr) {
        std::vector<std::string> modules = ModuleFactory::get().getPluginsList();
        for (const auto& module: modules) {
            const char* module_name = module.c_str();
            const char* module_metatable = (module + "_mt").c_str();

            int type = lua_getglobal(L, module_name);
            if (type != LUA_TNIL) {
                // Global already exists
                lua_pop(L, 1);
                continue;
            }

            // Create a new empty table
            lua_newtable(L);

            // Create the associated metatable
            luaL_newmetatable(L, module_metatable);

            lua_pushstring(L, module_name);
            lua_setfield(L, -2, "__name");

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

    /*!
     * Hook for the 'load_modules' lua function. The stack must have one element:
     *     1        (string) The filename of the library to load
     *
     * The library will be loaded, and for each module declared, a new global variable will
     * be declared, accessible in the lua configuration
     */
    int load_modules(lua_State* L) {
        void* cfg_ptr = lua_touserdata(L, lua_upvalueindex(1));

        const char *path = luaL_checkstring(L, 1);
        LibraryManager::get().registerLibrary(path);

        lua_register_modules(L, cfg_ptr);

        return 0;
    }
};

ConfigurationReader::ConfigurationReader(const std::string& file) {

    LOG(debug) << "Parsing LUA configuration from " << file;
    auto lua_state = luaL_newstate();

    luaL_openlibs(lua_state);

    // Register function load_modules
    lua_pushlightuserdata(lua_state, this);
    lua_pushcclosure(lua_state, lua::load_modules, 1);
    lua_setglobal(lua_state, "load_modules");

    // Register existing modules
    lua::lua_register_modules(lua_state, this);

    // Parse file
    if (luaL_dofile(lua_state, file.c_str())) {
        printf("%s\n", lua_tostring(lua_state, -1));
    }

    // Read global configuration from global variable named 'configuration'
    m_global_configuration.reset(new ConfigurationSet("configuration", "configuration"));
    int type = lua_getglobal(lua_state, "configuration");
    if (type == LUA_TTABLE) {
        LOG(debug) << "Parsing global configuration.";
        m_global_configuration->parse(lua_state, -1);
    }
    lua_pop(lua_state, 1);

    for (auto& m: m_light_modules) {
        LOG(debug) << "Configuration declared module " << m.type << "::" << m.name;

        lua_getglobal(lua_state, m.type.c_str());
        lua_getfield(lua_state, -1, m.name.c_str());

        m.parameters.reset(new ConfigurationSet(m.type, m.name, m_global_configuration));
        m.parameters->parse(lua_state, -1);

        lua_pop(lua_state, 2);
    }

    lua_close(lua_state);
}

void ConfigurationReader::addModule(const std::string& type, const std::string& name) {
    LightModule module;
    module.name = name;
    module.type = type;
    m_light_modules.push_back(module);
}

std::vector<LightModule> ConfigurationReader::getModules() const {
    return m_light_modules;
}
