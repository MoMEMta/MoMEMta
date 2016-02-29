#pragma once

#include <boost/any.hpp>
#include <lua.hpp>

namespace lua {

    enum Type {
        NOT_SUPPORTED,
        BOOLEAN,
        STRING,
        INTEGER,
        REAL,
        INPUT_TAG,
        CONFIGURATION_SET,
    };

    Type type(lua_State* L, int index);

    size_t get_index(lua_State* L, int index);

    /*!
     *  Check if a lua table is an array
     *
     *  Return -1 if it's not an array, or the size of the array
     */
    int lua_is_array(lua_State* L, int index);

    boost::any to_any(lua_State* L, int index);

    template<typename T> T special_any_cast(const boost::any& value) {
        return boost::any_cast<T>(value);
    }

    template<typename T>
    std::vector<T> lua_to_vectorT(lua_State* L, int index) {

        std::vector<T> result;

        size_t absolute_index = get_index(L, index);

        if (lua_type(L, absolute_index) != LUA_TTABLE)
            return result;

        lua_pushnil(L);
        while (lua_next(L, absolute_index) != 0) {
            result.push_back(special_any_cast<T>(to_any(L, -1)));
            lua_pop(L, 1);
        }

        return result;
    }

    boost::any lua_to_vector(lua_State* L, int index, Type type);
    boost::any lua_to_vector(lua_State* L, int index);
}
