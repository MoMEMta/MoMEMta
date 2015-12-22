#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <logging.h>

// Forward declaration
class Module;
class ConfigurationSet;
class Pool;

// Plugin system
template<typename T> class PluginFactory;

template<typename Interface, typename... Args>
class PluginFactory<Interface* (Args...)> {
    public:
        using type = Interface* (Args...);
        using Factory = PluginFactory<Interface*(Args...)>;

        struct PMakerBase {
            virtual std::shared_ptr<Interface> create(Args...) const = 0;
            virtual ~PMakerBase() {}
        };

        template<class ModuleType>
            struct PMaker: public PMakerBase {
                PMaker(const std::string& name) {
                    Factory::get().registerPMaker(this, name);
                }

                virtual std::shared_ptr<Interface> create(Args... args) const override {
                    return std::shared_ptr<ModuleType>(new ModuleType(std::forward<Args>(args)...));
                }
            };

        std::shared_ptr<Interface> create(const std::string& name, Args... args) const {
            return findPMaker(name)->create(std::forward<Args>(args)...);
        }

        static PluginFactory<Interface* (Args...)>& get();

        void registerPMaker(PMakerBase* pMaker, const std::string& name) {
            auto it = m_plugins.find(name);
            if (it != m_plugins.end())
                throw plugin_already_exists_error("The plugin type '" + name + "' is already registered in the factory.");

            LOG(trace) << "Registering plugin " << name << " in the factory";
            m_plugins.emplace(name, pMaker);
        }

        PMakerBase* findPMaker(const std::string& name) const {
            auto it = m_plugins.find(name);
            if (it == m_plugins.end())
                throw plugin_not_found_error("No such plugin type '" + name + "' registered in the factory.");

            return it->second;
        }

        std::vector<std::string> getPluginsList() const {
            std::vector<std::string> result;
            for (const auto& plugin: m_plugins) {
                result.push_back(plugin.first);
            }

            return result;
        }

    private:
        class plugin_already_exists_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        class plugin_not_found_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        PluginFactory() = default;

        PluginFactory(const PluginFactory&) = delete; // stop default
        const PluginFactory& operator=(const PluginFactory&) = delete; // stop default

        std::unordered_map<std::string, PMakerBase*> m_plugins;
};

using ModuleFactory = PluginFactory<Module* (std::shared_ptr<Pool>, const ConfigurationSet&)>;

#define MODULE_UNIQUE_NAME2(x, y) x ## y
#define MODULE_UNIQUE_NAME(x, y) MODULE_UNIQUE_NAME2(x, y)
#define REGISTER_MODULE(type) \
    static const ModuleFactory::PMaker<type> MODULE_UNIQUE_NAME(s_module , __LINE__)(#type)
