/*
 *  MoMEMta: a modular implementation of the Matrix Element Method
 *  Copyright (C) 2017  Universite catholique de Louvain (UCL), Belgium
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

#include <mutex>
#include <functional>

#include <momemta/ModuleFactory.h>
#include <momemta/ModuleDefBuilder.h>

namespace momemta {

class ModuleRegistryInterface {
public:
    virtual ~ModuleRegistryInterface();

    /// Returns the registration data for a given module. Throws an exception if no such module exists.
    virtual const ModuleRegistrationData& find(const std::string& module_name) const = 0;
};

/**
 * A registry containing a list of all available modules, along with registration data detailing modules' inputs, outputs
 * and attributes.
 *
 * Thread-safe.
 */
class ModuleRegistry: ModuleRegistryInterface {

public:
    typedef std::function<ModuleRegistrationData()> RegisterOp;

    ModuleRegistry();
    virtual ~ModuleRegistry() {}

    /// A singleton available at startup
    static ModuleRegistry& get();

    void registerModule(RegisterOp registration_op);

    /**
     * Deregister a module from the registry. If no such module is registered, nothing happens
     *
     * \param module_name The name of the module to deregister
     */
    void deregisterModule(const std::string& module_name);

    /**
     * Find registration data of the module \p module_name. If no such module exists, an exception is thrown
     * \param module_name The name of the module
     * \return Registration data associated to the module
     */
    const ModuleRegistrationData& find(const std::string& module_name) const override;

    /**
     * Fill \p list with the list of all registered modules. Internal modules are ignored if \p ignore_internal is true
     *
     * \param ignore_internal If set to true, internal modules are ignored when filling the list
     * \param list List of all registered modules. The list is cleared first.
     */
    void exportList(bool ignore_internal, ModuleList& list) const;

    /**
     * Process the current list of deferred registration
     */
    void processRegistrations();

private:

    void callDeferred() const;
    void registerModuleWithLock(RegisterOp registration_op) const;

    mutable std::mutex mutex_;

    /**
     * List of registration operation, allowing deferred registration.
     * Actual registration happens only when needed (first lookup or export)
     */
    mutable std::vector<RegisterOp> deferred_; // Guarded by mutex_
    mutable std::unordered_map<std::string, const ModuleRegistrationData> registry_; // Guarded by mutex_

    mutable bool initialized_ = false; // Guarded by mutex_

    // Exceptions
    class module_already_exists_error: public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    class module_not_found_error: public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
};

namespace registration {

struct ModuleDefBuilderReceiver {
    // To call ModuleRegistry::get()->register(...), used by the
    // REGISTER_MODULE macro below.
    // Note: These are implicitly converting constructors.
    ModuleDefBuilderReceiver(const ModuleDefBuilder& builder);

    ~ModuleDefBuilderReceiver();

    /// \private
    std::string name_;
};

}

#define REGISTER_MODULE(type) \
    REGISTER_MODULE_UNIQ_HELPER(__LINE__, #type, type)
#define REGISTER_MODULE_NAME(name, type) \
    REGISTER_MODULE_UNIQ_HELPER(__LINE__, name, type)

#define REGISTER_MODULE_UNIQ_HELPER(ctr, name, type) REGISTER_MODULE_UNIQ(ctr, name, type)
#define REGISTER_MODULE_UNIQ(ctr, name, type) \
  static const ::momemta::ModuleFactory::PMaker<type> register_module_factory##ctr(name); \
  static const ::momemta::registration::ModuleDefBuilderReceiver register_module##ctr = \
          ::momemta::registration::ModuleDefBuilder(name).Type<type>()
}
