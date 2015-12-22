#pragma once

#include <memory>
#include <string>
#include <vector>

#include <ConfigurationSet.h>

struct LightModule {
    std::string name;
    std::string type;
    std::shared_ptr<ConfigurationSet> parameters;
};

class ConfigurationReader {
    public:
        ConfigurationReader(const std::string&);

        void addModule(const std::string& type, const std::string& name);

        std::vector<LightModule> getModules() const;

    private:
        std::vector<LightModule> m_light_modules;
        std::shared_ptr<ConfigurationSet> m_global_configuration;
};
