#pragma once

#include <boost/any.hpp>
#include <lua.hpp>

/// Utility functions related to lua configuration file parsing
namespace lua {

    /**
     * \brief List of all supported lua types
     */
    enum Type {
        NOT_SUPPORTED, /**< Type not supported */
        BOOLEAN, /**< Map to `bool` */
        STRING, /**< Map to `std::string` */
        INTEGER, /**< Map to `int64_t` */
        REAL, /**< Map to `double` */
        INPUT_TAG, /**< Map to InputTag */
        CONFIGURATION_SET, /**< Map to ConfigurationSet */
    };

    /**
     * \brief Extract the type of a lua value
     *
     * \param L The current lua state
     * \param index The index of the value
     *
     * \return The type of the lua value (can be ::NOT_SUPPORTED if the framework can't handle such value)
     */
    Type type(lua_State* L, int index);

    /**
     * \brief Convert a negative lua stack index to an absolute index
     *
     * \param L The current lua state
     * \param index The index to convert
     *
     * \return `index` if it's positive, otherwise the absolute index obtained by `lua_gettop(L) + index + 1`
     */
    size_t get_index(lua_State* L, int index);

    /*!
     *  \brief Check if a lua table is an array
     *
     *  \return -1 if it's not an array, or the size of the array
     */
    int lua_is_array(lua_State* L, int index);

    /**
     * \brief Convert a lua type to boost::any
     *
     * \return A new boost::any encapsulating the lua type. If the lua type is not supported, an exception is thrown
     */
    boost::any to_any(lua_State* L, int index);

    template<typename T> T special_any_cast(const boost::any& value) {
        return boost::any_cast<T>(value);
    }

    /**
     * \brief Convert a lua array to a typed vector, encapsulated into a boost::any
     *
     * \return A new boost::any encapsulating the lua array. If the lua type is not supported, an exception is thrown
     */
    template<typename T>
    boost::any to_vectorT(lua_State* L, int index) {

        std::vector<T> result;

        size_t absolute_index = get_index(L, index);

        if (lua_type(L, absolute_index) != LUA_TTABLE)
            return result;

        lua_pushnil(L);
        while (lua_next(L, absolute_index) != 0) {
            result.push_back(special_any_cast<T>(to_any(L, -1)));
            lua_pop(L, 1);
        }

        return boost::any(result);
    }

    /**
     * \brief Convert a lua array to a typed vector, encapsulated into a boost::any
     *
     * \return A new boost::any encapsulating the lua array. If the lua type is not supported, an exception is thrown
     */
    boost::any to_vector(lua_State* L, int index, Type type);
}
