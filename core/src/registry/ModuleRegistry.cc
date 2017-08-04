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

#include <momemta/Logging.h>
#include <momemta/ModuleRegistry.h>

namespace momemta {

ModuleRegistryInterface::~ModuleRegistryInterface() {}

ModuleRegistry& ModuleRegistry::get() {
    static ModuleRegistry s_instance;
    return s_instance;
}

ModuleRegistry::ModuleRegistry() { }

void ModuleRegistry::registerModule(RegisterOp registration_op) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!initialized_) {
        deferred_.emplace_back(registration_op);
    } else {
        registerModuleWithLock(registration_op);
    }
}

void ModuleRegistry::deregisterModule(const std::string& module_name) {
    std::unique_lock<std::mutex> lock(mutex_);
    callDeferred();

    registry_.erase(module_name);
}

const ModuleRegistrationData& ModuleRegistry::find(const std::string& module_name) const {

    const ModuleRegistrationData* data = nullptr;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        callDeferred();

        auto it = registry_.find(module_name);
        if (it != registry_.end())
            data = &it->second;
    }

    if (! data) {
        throw module_not_found_error("Module '" + module_name + "' is not present in the registry");
    }

    return *data;
}

void ModuleRegistry::exportList(bool ignore_internal, ModuleList& list) const {
    list.clear();

    std::unique_lock<std::mutex> lock(mutex_);
    callDeferred();

    for (const auto& it: registry_) {
        if (ignore_internal && it.second.module_def.internal)
            continue;

        list.emplace_back(it.second.module_def);
    }
}

void ModuleRegistry::processRegistrations() {
    std::unique_lock<std::mutex> lock(mutex_);
    callDeferred();
}

void ModuleRegistry::callDeferred() const {
    if (initialized_)
        return;

    initialized_ = true;

    for (auto& fn: deferred_) {
        registerModuleWithLock(fn);
    }

    deferred_.clear();
}

void ModuleRegistry::registerModuleWithLock(RegisterOp registration_op) const {
    auto registration_data = registration_op();

    // Insert module into registry
    auto it = registry_.find(registration_data.module_def.name);
    if (it != registry_.end())
        throw module_already_exists_error("The module '" + registration_data.module_def.name + "' already exists.");

    registry_.emplace(registration_data.module_def.name, registration_data);
}

namespace registration {


ModuleDefBuilderReceiver::ModuleDefBuilderReceiver(const ModuleDefBuilder& builder) {
    name_ = builder.name();
    ModuleRegistry::get().registerModule([builder]() -> ModuleRegistrationData {
        return builder.Build();
    });
}

ModuleDefBuilderReceiver::~ModuleDefBuilderReceiver() {
    ModuleRegistry::get().deregisterModule(name_);
}

}
}