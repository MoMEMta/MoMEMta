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


#pragma once

#include <map>
#include <memory>
#include <string>

#include <momemta/InputTag.h>
#include <momemta/impl/traits.h>
#include <momemta/Logging.h>

#include <boost/any.hpp>

struct lua_State;
class ConfigurationReader;
class Configuration;

/**
 * \brief A class encapsulating a lua table.
 *
 * All the items inside the table will be converted to plain C++ type, and can be accessed via the ParameterSet::get() functions.
 *
 * **Only** the following types are supported:
 *
 *  - `int64_t`
 *  - `double`
 *  - `bool`
 *  - `std::string`
 *  - `InputTag`
 *  - `ParameterSet`
 *  - `std::vector<>` of one of the above types
 *
 * Consider the following lua snippet:
 *
 * ```lua
 * {
 *      x = 91.,
 *      y = "string",
 *      z = {10, 20, 30, 40}
 * }
 * ```
 *
 * The resulting `ParameterSet` will encapsulates three values:
 *   - `x` of type `double` = `91.`
 *   - `y` of type `std::string` = `"string"`
 *   - `z` of type `std::vector<int64_t>` = `[10, 20, 30, 40]`
 *
 * and you would use the following code to access the `z` value:
 *
 * ```
 * std::vector<int64_t> values = set.get<std::vector<int64_t>>("z");
 * ```
 *
 * \todo Document the freezing of the configuration in ConfigurationReader
 *
 * \note You should *never* try to create a ParameterSet yourself. Always use the ConfigurationReader class to parse the configuration file
 *
 */
class ParameterSet {
    public:
        ParameterSet() = default;

        template<typename T> const T& get(const std::string& name) const {
            auto value = m_set.find(name);
            if (value == m_set.end())
                throw not_found_error("Parameter '" + name + "' not found.");

            return boost::any_cast<const T&>(value->second.value);
        }

        template<typename T> const T& get(const std::string& name, const T& defaultValue) const {
            auto value = m_set.find(name);
            if (value == m_set.end())
                return defaultValue;

            return boost::any_cast<const T&>(value->second.value);
        }

        bool exists(const std::string& name) const;
        template<typename T> bool existsAs(const std::string& name) const {
            auto value = m_set.find(name);
            return (value != m_set.end() && value->second.value.type() == typeid(T));
        }

        /**
         * \brief Change the value of a given parameter. If the parameter does not exist, it's first created.
         *
         * \param name The name of the parameter to change
         * \param value The new value of the parameter
         *
         * \warning Vectors are currently not supported
         */
        template<typename T>
        typename std::enable_if<std::is_same<T, bool>::value ||
                                std::is_same<T, InputTag>::value>::type set(const std::string& name, const T& value) {
            set_helper(name, value);
        }

        /**
         * \brief Change the value of a given parameter. If the parameter does not exist, it's first created.
         *
         * Specialization for string type, with implicit cast to `std::string`
         *
         * \param name The name of the parameter to change
         * \param value The new value of the parameter
         *
         * \warning Vectors are currently not supported
         */
        template<typename T>
        typename std::enable_if<is_string<T>::value>::type set(const std::string& name, const T& value) {
            set_helper(name, std::string(value));
        }

        /**
         * \brief Change the value of a given parameter. If the parameter does not exist, it's first created.
         *
         * Specialization for integral type, with implicit cast to `int64_t`
         *
         * \param name The name of the parameter to change
         * \param value The new value of the parameter
         *
         * \warning Vectors are currently not supported
         */
        template<typename T>
        typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type set(const std::string& name, const T& value) {
            set_helper(name, static_cast<int64_t>(value));
        }

        /**
         * \brief Change the value of a given parameter. If the parameter does not exist, it's first created.
         *
         * Specialization for floating-point type, with implicit cast to `double`
         *
         * \param name The name of the parameter to change
         * \param value The new value of the parameter
         *
         * \warning Vectors are currently not supported
         */
        template<typename T>
        typename std::enable_if<std::is_floating_point<T>::value>::type set(const std::string& name, const T& value) {
            set_helper(name, static_cast<double>(value));
        }

        /**
         * \brief Parse fields of a lua table
         *
         * \param L the lua state
         * \param index The index on the stack of the table to parse
         */
        void parse(lua_State* L, int index);

        std::string getModuleName() const {
            return get<std::string>("@name", "");
        }

        std::string getModuleType() const {
            return get<std::string>("@type", "");
        }

        const ParameterSet& globalParameters() const {
            auto it = m_set.find("@global_parameters");
            if (it == m_set.end())
                return *this;

            return boost::any_cast<const ParameterSet&>(it->second.value);
        }

        /**
         * \brief Clone this ParameterSet
         *
         * You must take ownership of the returned pointer.
         *
         * @return A clone of this ParameterSet. You must take ownership of the returned value.
         */
        virtual ParameterSet* clone() const;

    protected:
        friend class ConfigurationReader;
        friend class Configuration;

        /// A small wrapper around a boost::any value
        struct Element {
            boost::any value;
            bool lazy = false; /// If true, it means we hold a lazy value which should be evaluated

            template <typename T>
            Element(const T& v) {
                value = v;
            }

            template <typename T>
            Element(const T& v, bool l) {
                value = v;
                lazy = l;
            }
        };

        ParameterSet(const std::string& module_type, const std::string& module_name);

        virtual std::pair<boost::any, bool> parseItem(const std::string& key, lua_State* L, int index);

        /**
         * \brief Add a new element to the ParameterSet
         *
         * \param name Name of the new parameter
         * \param value Value of the new parameter
         */
        virtual void create(const std::string& name, const boost::any& value);
        virtual void setInternal(const std::string& name, Element& element, const boost::any& value);

        virtual void freeze();

        std::map<std::string, Element> m_set;

    private:

        class not_found_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        class frozen_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        void setGlobalParameters(const ParameterSet&);

        /**
         * \brief Change the value of a given parameter. If the parameter does not exist, it's first created.
         *
         * \param name The name of the parameter to change
         * \param value The new value of the parameter
         *
         * \warning Vectors are currently not supported
         */
        template<typename T>
        void set_helper(const std::string& name, const T& value) {
            static_assert(
                    std::is_same<T, int64_t>::value ||
                    std::is_same<T, double>::value ||
                    std::is_same<T, bool>::value ||
                    std::is_same<T, std::string>::value ||
                    std::is_same<T, InputTag>::value,
                    "Type not supported"
            );

            if (frozen) {
                LOG(fatal) << "You are not allowed to edit a set once frozen.";
                throw frozen_error("This ParameterSet is frozen");
            }

            auto it = m_set.find(name);
            // If the element does not exist in the set, we create it
            // Otherwise, we simply update the value
            if (it == m_set.end()) {
                create(name, value);
            } else {
                setInternal(name, it->second, value);
            }
        }

        bool frozen = false;
};

/** \brief A lazy parameter set
 *
 * This class is used to represent global tables that can be modified **after** the parsing of the configuration file (like the global `parameters` table). This means that each field of the table *must* have a delayed evaluation (see lua::LazyTableField).
 *
 * Actual evaluation of the fields of the table happens during the freezing of this ParameterSet.
 *
 */

class lua_State;

class LazyParameterSet: public ParameterSet {
    friend class ConfigurationReader;
    public:
        LazyParameterSet(std::shared_ptr<lua_State> L, const std::string& name);
        virtual LazyParameterSet* clone() const override;

    protected:
        virtual std::pair<boost::any, bool> parseItem(const std::string& key, lua_State* L, int index) override;

        virtual void create(const std::string& name, const boost::any& value) override;
        virtual void setInternal(const std::string& name, Element& element, const boost::any& value) override;

        virtual void freeze() override;

    private:
        std::shared_ptr<lua_State> m_lua_state;
};
