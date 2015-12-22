#include <ConfigurationSet.h>
#include <logging.h>
#include <lua/utils.h>

void ConfigurationSet::parse(lua_State* L, int index) {

    LOG(trace) << "[parse] >> stack size = " << lua_gettop(L);
    size_t absolute_index = lua::get_index(L, index);

    lua_pushnil(L);
    while (lua_next(L, absolute_index) != 0) {

        std::string key = lua_tostring(L, -2);

        LOG(trace) << "[parse] >> key = " << key;

        try {
            boost::any value = lua::to_any(L, -1);
            m_set.emplace(key, value);
        } catch(...) {
            LOG(emerg) << "Exception while trying to parse parameter " << m_module_type << "." << m_module_name << "::" << key;
            std::rethrow_exception(std::current_exception());
        }

        lua_pop(L, 1);
    }

    LOG(trace) << "[parse] << stack size = " << lua_gettop(L);
}

bool ConfigurationSet::exists(const std::string& name) const {
    auto value = m_set.find(name);
    return (value != m_set.end());
}
