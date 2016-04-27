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

#include <boost/any.hpp>
#include <lua/utils.h>
#include <map>
#include <memory>
#include <string>

class ConfigurationReader;
class Configuration;

/**
 * \brief A class encapsulating a lua table.
 *
 * All the items inside the table will be converted to plain C++ type, and can be accessed via the ConfigurationSet::get() functions.
 *
 * **Only** the following types are supported:
 *
 *  - `int64_t`
 *  - `double`
 *  - `bool`
 *  - `std::string`
 *  - `InputTag`
 *  - `ConfigurationSet`
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
 * The resulting `ConfigurationSet` will encapsulates three values:
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
 * \note You should *never* try to create a ConfigurationSet yourself. Always use the ConfigurationReader class to parse the configuration file
 *
 */
class ConfigurationSet {
    public:
        ConfigurationSet() = default;

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
         * \brief Change the value of a given parameter
         *
         * \param name The name of the parameter to change
         * \param value The new value of the parameter
         *
         * \note The parameter must already exist
         *
         * \warning Vectors are currently not supported
         */
        template<typename T>
        void set(const std::string& name, const T& value) {
            static_assert(
                    std::is_same<T, int64_t>::value ||
                    std::is_same<T, double>::value ||
                    std::is_same<T, bool>::value ||
                    std::is_same<T, std::string>::value,
                    "Type not supported"
            );

            if (frozen) {
                LOGGER->critical("You are not allowed to edit a set once frozen.");
                throw frozen_error("This ConfigurationSet is frozen");
            }

            auto it = m_set.find(name);
            if (it == m_set.end()) {
                LOGGER->critical("Parameter '{}' not found. You can only set values that already exist.", name);
                throw not_found_error("Parameter " + name + " not found");
            }

            setInternal(name, it->second, value);
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

        const ConfigurationSet& globalConfiguration() const {
            auto it = m_set.find("@global_configuration");
            if (it == m_set.end())
                return *this;

            return boost::any_cast<const ConfigurationSet&>(it->second.value);
        }

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

        ConfigurationSet(const std::string& module_type, const std::string& module_name);

        virtual std::pair<boost::any, bool> parseItem(const std::string& key, lua_State* L, int index);

        virtual void setInternal(const std::string& name, Element& element, const boost::any& value);

    private:

        class not_found_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        class frozen_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        friend std::pair<boost::any, bool> lua::to_any(lua_State* L, int index);

        void setGlobalConfiguration(const ConfigurationSet& configuration);

        void freeze();

        bool frozen = false;
        std::map<std::string, Element> m_set;
};

/** \brief A lazy configuration set
 *
 * This class is used to represent global tables that can be modified **after** the parsing of the configuration file (like the global `parameters` table). This means that each field of the table *must* have a delayed evaluation (see lua::LazyTableField).
 *
 * Actual evaluation of the fields of the table happens during the freezing of this ConfigurationSet.
 *
 */
class LazyConfigurationSet: public ConfigurationSet {
    friend class ConfigurationReader;

    protected:
        LazyConfigurationSet(const std::string& name);

        virtual std::pair<boost::any, bool> parseItem(const std::string& key, lua_State* L, int index) override;

        virtual void setInternal(const std::string& name, Element& element, const boost::any& value) override;
};
