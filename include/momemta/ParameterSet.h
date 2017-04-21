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

#include <momemta/any.h>
#include <momemta/InputTag.h>
#include <momemta/impl/traits.h>
#include <momemta/Logging.h>
#include <momemta/Utils.h>

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
 * The resulting `ParameterSet` will encapsulate three values:
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
 */
class ParameterSet {
    public:
        ParameterSet() = default;

        template<typename T> const T& get(const std::string& name) const {
            auto value = m_set.find(name);
            if (value == m_set.end())
                throw not_found_error("Parameter '" + name + "' not found.");

            try {
                return momemta::any_cast<const T&>(value->second.value);
            } catch (momemta::bad_any_cast e) {
                LOG(fatal) << "Exception while trying to get parameter '" << name << "'. Requested a '"
                           << demangle(typeid(T).name())
                           << "' while parameter is a '"
                           << demangle(value->second.value.type().name())
                           << "'";
                throw e;
            }
        }

        template<typename T> const T& get(const std::string& name, const T& defaultValue) const {
            auto value = m_set.find(name);
            if (value == m_set.end())
                return defaultValue;

            try {
                return momemta::any_cast<const T&>(value->second.value);
            } catch (momemta::bad_any_cast e) {
                LOG(fatal) << "Exception while trying to get parameter '" << name << "'. Requested a '"
                           << demangle(typeid(T).name())
                           << "' while parameter is a '"
                           << demangle(value->second.value.type().name())
                           << "'";
                throw e;
            }
        }

        /**
         * Retrieve a raw value from this ParameterSet.
         *
         * @param name The name of the parameter
         * @return The raw value of the parameter. A `not_found_error` exception is thrown if \p parameter does not
         * exist in this set.
         */
        const momemta::any& rawGet(const std::string& name) const;

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
         */
        template<typename T>
        typename std::enable_if<std::is_same<T, bool>::value ||
                                std::is_same<T, InputTag>::value>::type set(const std::string& name, const T& value) {
            set_helper(name, value);
        }
        template<typename T>
        typename std::enable_if<std::is_same<T, bool>::value ||
                                std::is_same<T, InputTag>::value>::type set(const std::string& name, const std::vector<T>& value) {
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

            return momemta::any_cast<const ParameterSet&>(it->second.value);
        }

        /**
         * \brief Clone this ParameterSet
         *
         * You must take ownership of the returned pointer.
         *
         * @return A clone of this ParameterSet. You must take ownership of the returned value.
         */
        virtual ParameterSet* clone() const;

        std::vector<std::string> getNames() const;

    protected:
        friend class ConfigurationReader;
        friend class Configuration;
        friend class ParameterSetParser;

        /// A small wrapper around a momemta::any value
        struct Element {
            momemta::any value;
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

        /**
         * \brief A flag indicating if this ParameterSet lazy loads its fields or not
         *
         * \return True if doing lazy loading, false otherwise
         */
        virtual bool lazy() const;

        /**
         * \brief Add a new element to the ParameterSet
         *
         * \param name Name of the new parameter
         * \param value Value of the new parameter
         */
        virtual void create(const std::string& name, const momemta::any& value);
        virtual void setInternal(const std::string& name, Element& element, const momemta::any& value);

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
                    std::is_same<T, InputTag>::value ||
                    std::is_same<T, std::vector<int64_t>>::value ||
                    std::is_same<T, std::vector<double>>::value ||
                    std::is_same<T, std::vector<bool>>::value ||
                    std::is_same<T, std::vector<std::string>>::value ||
                    std::is_same<T, std::vector<InputTag>>::value,
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
