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

class ConfigurationSet {
    public:
        template<typename T> const T& get(const std::string& name) const {
            auto value = m_set.find(name);
            if (value == m_set.end())
                throw not_found_error("Parameter '" + name + "' not found.");

            return boost::any_cast<const T&>(value->second);
        }

        template<typename T> const T& get(const std::string& name, const T& defaultValue) const {
            auto value = m_set.find(name);
            if (value == m_set.end())
                return defaultValue;

            return boost::any_cast<const T&>(value->second);
        }

        bool exists(const std::string& name) const;
        template<typename T> bool existsAs(const std::string& name) const {
            auto value = m_set.find(name);
            return (value != m_set.end() && value->second.type() == typeid(T));
        }

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

            return *boost::any_cast<std::shared_ptr<ConfigurationSet>>(it->second);
        }

    private:
        class not_found_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        friend class ConfigurationReader;
        friend boost::any lua::to_any(lua_State* L, int index);

        ConfigurationSet() = default;
        ConfigurationSet(std::shared_ptr<ConfigurationSet> globalConfiguration);
        ConfigurationSet(const std::string& module_type, const std::string& module_name);
        ConfigurationSet(const std::string& module_type, const std::string& module_name, std::shared_ptr<ConfigurationSet> globalConfiguration);

        std::map<std::string, boost::any> m_set;
};
