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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <logging.h>

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

        template<class PluginType>
            struct PMaker: public PMakerBase {
                PMaker(const std::string& name) {
                    Factory::get().registerPMaker(this, name);
                }

                virtual std::shared_ptr<Interface> create(Args... args) const override {
                    return std::shared_ptr<PluginType>(new PluginType(std::forward<Args>(args)...));
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

            LOGGER->debug("Registering plugin '{}' in the factory ({:x})", name, (std::uintptr_t) this);
            m_plugins.emplace(name, pMaker);
        }

        PMakerBase* findPMaker(const std::string& name) const {
            auto it = m_plugins.find(name);
            if (it == m_plugins.end()) {
                LOGGER->critical("No such plugin type '{}' registered in the factory ({:x}), {} plugins.", name, (std::uintptr_t) this, m_plugins.size());
                throw plugin_not_found_error("No such plugin type '" + name + "' registered in the factory.");
            }

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

#define PLUGIN_UNIQUE_NAME2(x, y) x ## y
#define PLUGIN_UNIQUE_NAME(x, y) PLUGIN_UNIQUE_NAME2(x, y)
