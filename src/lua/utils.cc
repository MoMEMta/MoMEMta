#include <stdexcept>
#include <logging.h>

#include <lua/utils.h>
#include <InputTag.h>
#include <ConfigurationSet.h>

namespace lua {

    class invalid_array_error: public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    Type type(lua_State* L, int index) {
        int t = lua_type(L, index);

        Type result = NOT_SUPPORTED;
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
     *  Check if a lua table contains only value from the same type
     *
     *  Return -1 if not, or the type of the data in the table
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

    boost::any to_any(lua_State* L, int index) {
    
        LOG(trace) << "[to_any] >> stack size = " << lua_gettop(L);
        size_t absolute_index = get_index(L, index);

        boost::any result;

        switch (lua_type(L, absolute_index)) {
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
                        result = lua_to_vector(L, absolute_index, type);
                    } else {
                        // Convert to a vector of boost::any
                        // result = lua_to_vector(L, absolute_index);
                        throw invalid_array_error("Various types stored into the array. This is not supported.");
                    }

                } else {
                    ConfigurationSet cfg("", "");
                    cfg.parse(L, absolute_index);
                    result = cfg;
                }
                LOG(trace) << "[to_any::table] << stack size = " << lua_gettop(L);
            } break;
        }

        LOG(trace) << "[to_any] << final type = " << result.type().name();
        LOG(trace) << "[to_any] << stack size = " << lua_gettop(L);
        return result;
    }

    boost::any lua_to_vector(lua_State* L, int index, Type t) {
        switch (t) {
            case BOOLEAN:
                return boost::any(lua_to_vectorT<bool>(L, index));

            case STRING:
                return boost::any(lua_to_vectorT<std::string>(L, index));

            case INTEGER:
                return boost::any(lua_to_vectorT<int64_t>(L, index));

            case REAL:
                return boost::any(lua_to_vectorT<double>(L, index));

            case INPUT_TAG:
                return boost::any(lua_to_vectorT<InputTag>(L, index));

            case CONFIGURATION_SET:
                return boost::any(lua_to_vectorT<ConfigurationSet>(L, index));
        }

        throw invalid_array_error("Unsupported array type");
    }

    boost::any lua_to_vector(lua_State* L, int index) {
        std::vector<boost::any> result;

        size_t absolute_index = get_index(L, index);

        if (lua_type(L, absolute_index) != LUA_TTABLE)
            return result;

        lua_pushnil(L);
        while (lua_next(L, absolute_index) != 0) {
            result.push_back(to_any(L, -1));
            lua_pop(L, 1);
        }

        return result;
    }

    //! Specialization for double type, with implicit conversion from integer
    template<> double special_any_cast(const boost::any& value) {
        if (value.type() == typeid(int64_t))
            return static_cast<double>(boost::any_cast<int64_t>(value));

        return boost::any_cast<double>(value);
    }
}
