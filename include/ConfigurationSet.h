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
            return m_module_name;
        }

        std::string getModuleType() const {
            return m_module_type;
        }

        const ConfigurationSet& globalConfiguration() const {
            if (m_global_configuration.get())
                return *m_global_configuration;
            else
                return *this;
        }

    private:
        class not_found_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        friend class ConfigurationReader;
        friend boost::any lua::to_any(lua_State* L, int index);


        ConfigurationSet(const std::string& module_type, const std::string& module_name):
            m_module_type(module_type),
            m_module_name(module_name) {}

        ConfigurationSet(const std::string& module_type, const std::string& module_name, std::shared_ptr<ConfigurationSet> globalConfiguration): ConfigurationSet(module_type, module_name) {
            m_global_configuration = globalConfiguration;
        }

        std::string m_module_type;
        std::string m_module_name;
        std::map<std::string, boost::any> m_set;
        std::shared_ptr<ConfigurationSet> m_global_configuration;
};
