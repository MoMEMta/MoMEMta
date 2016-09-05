#pragma once

#include <memory>
#include <stdexcept>
#include <tuple>

#include <boost/any.hpp>
#include <lua.hpp>

class ILuaCallback;

/*! \brief Utility functions related to lua configuration file parsing
 *
 * The lua configuration file specify a set of modules to execute.
 *
 * \todo describe how the configuration file is parsed and what it structure is
 *
 */
namespace lua {

    ///< Thrown if the configuration file is not valid
    class invalid_configuration_file: public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    class invalid_array_error: public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    class unsupported_type_error: public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

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
        PARAMETER_SET, /**< Map to ParameterSet */
    };

    /**
     * \brief Lazy value in lua (delayed evaluation)
     *
     * Use Lazy::operator()() to evaluate the value
     */
    struct Lazy {
        lua_State* L; ///< The global lua state. This state must be valid for as long as this instance.

        Lazy(lua_State* L);

        /**
         * \brief Evaluate the lazy value
         *
         * \return The value after evaluation
         */
        virtual boost::any operator() () const = 0;
    };

    /**
     * \brief Lazy function in lua (delayed function evaluation)
     *
     * It's just a small wrapper around a lua anonymous function.
     */
    struct LazyFunction: public Lazy {
        int ref_index; ///< The reference index where the anonymous function is stored.

        /**
         * \brief Evaluate the anonymous function
         *
         * \return The return value of the anonymous function
         */
        virtual boost::any operator() () const override;

        /**
         * \brief Bind a anonymous lua function
         *
         * \param L The current lua state.
         * \param index The index of the anonymous function on the stack
         */
        LazyFunction(lua_State* L, int index);
    };

    /**
     * \brief Lazy table field in lua (delayed table access)
     *
     * It's a wrapper around table's field access. Evaluation of this lazy value means accessing the field of the table.
     */
    struct LazyTableField: public Lazy {
        std::string table_name; ///< The name of the global table
        std::string key; ///< The name of the field inside the table to retrieve when evaluated

        virtual boost::any operator() () const override;

        /**
         * \brief Ensure the global table referenced by this `struct` exist. If not, create it.
         */
        void ensure_created();

        /**
         * \brief Replace the value of the table field by a new one
         */
        void set(const boost::any& value);

        LazyTableField(lua_State* L, const std::string& table_name, const std::string& key);
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
     * \return A std::pair containing :
     *   - a new boost::any encapsulating the lua type and a boolean. If the lua type is not supported, an exception is thrown.
     *   - a boolean. If true, it means the encapsulated type is a Lazy value, which must be evaluated later on.
     */
    std::pair<boost::any, bool> to_any(lua_State* L, int index);

    /**
     * \brief Convert a boost::any to a lua type, and push it to the top of the stack
     *
     * This method does the opposite of lua::to_any: convert a boost::any to the corresponding lua type, and pushing it
     * to the top of the stack.
     *
     * \param L the current lua state
     * \param value The value to convert
     *
     * \warning Vectors are currently not supported
     */
    void push_any(lua_State* L, const boost::any& value);

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
            boost::any value;
            bool lazy = false;
            std::tie(value, lazy) = to_any(L, -1);
            result.push_back(special_any_cast<T>(value));
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

    /*!
     *  \brief Register all C function in the lua userspace.
     *
     *  Available functions:
     *      - @link load_modules @endlink
     *      - @link parameter @endlink
     */
    void setup_hooks(lua_State* L, void* ptr);

    /*!
     * \brief Hook for the metatable `__newindex` of the module's table
     *
     * This function is called as soon as a new entry is added to a module's table, which means
     * that the user defines a new module from the configuration.
     *
     * The stack contains 3 elements:
     *  1. the module type table
     *  2. the module name (the key in the table)
     *  3. the module definition (the value associated with the key)
     *
     *  \return always 0
     */
    int module_table_newindex(lua_State* L);

    /**
     * \brief Register modules in lua userspace
     *
     * For each type of module existing, a global `table` named as
     * the module's type is declared.
     *
     * Each entry in this table declare a new module of a given type. The key
     * of the entry is the module's name, and the name of the table is the module's type.
     *
     * The value is itself a table, defining the configuration of the module
     *
     * \param L The current lua state
     * \param ptr A pointer to an instance of ConfigurationReader
     *
     * \sa module_table_newindex()
     */
    void register_modules(lua_State* L, void* ptr);

    /*!
     * \brief Hook for the `load_modules` lua function. The stack must have one element:
     *   1. (string) The filename of the library to load
     *
     * The library will be loaded, and for each new module, a new global variable will
     * be declared, accessible from the lua configuration
     *
     * \return always 0
     */
    int load_modules(lua_State* L);

    /*!
     * \brief Hook for the `parameter` lua function. This function accepts one argument:
     *   1. (string) The name of the parameter
     *
     * Internaly, this function creates an anonymous lua function, which returns the value of
     * the parameter from the `configuration` global table.
     *
     * \return always 1
     */
    int parameter(lua_State* L);

    /** \brief Initialize the lua runtime
     *
     * \param callback A pointer to an instance of ILuaCallback. This callback is used for
     * communication between lua and C++
     *
     * \return A pointer to the global lua state. Once out-of-scope, the pointer will be
     * released using the `lua_close` function.
     */
    std::shared_ptr<lua_State> init_runtime(ILuaCallback* callback);
   
    /** \brief Define Lua function to generate Cuba phase-space point input-tags
     *
     * \param L The current lua state
     *
     * The Lua function returns an input tag of type `cuba::ps_points/i` where `i` gets incremented each time the function is called (starting from 0).
     * This way, the user is sure to always define the correct input tag fothe phase-space points.
     *
     * \return always 1
     */
    int generate_cuba_inputtag(lua_State* L);
}
