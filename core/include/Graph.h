#pragma once

#include <momemta/Configuration.h>
#include <momemta/Module.h>

#include <ExecutionPath.h>

#include <map>
#include <string>
#include <vector>

#include <boost/functional/hash.hpp>

/// Generic graph representation of the module hierarchy
namespace graph {

class SortedModuleList {

public:
    void addModule(const boost::uuids::uuid& path, const Configuration::ModuleDecl& module);
    const std::vector<boost::uuids::uuid>& getPaths() const;
    const std::vector<Configuration::ModuleDecl>& getModules(const boost::uuids::uuid& path) const;

private:
    std::unordered_map<
            boost::uuids::uuid,
            std::vector<Configuration::ModuleDecl>,
            boost::hash<boost::uuids::uuid>
    > modules;

    std::vector<boost::uuids::uuid> sorted_execution_paths;
};

/**
 * \brief Sort the list of modules declared in the configuration according to their dependencies.
 *
 * Modules are sorted based on their inputs and outputs dependencies, ensuring modules producing quantities
 * are executed before modules depending on these quantities. If a module does not contribute, it's ignored
 * and won't be present in the sorted list.
 *
 * If sorting is not possible (cyclic dependencies for example), an exception is thrown
 *
 * \param[in] available_modules List of all available modules, with their definition.
 * \param[in] requested_modules List of modules to sort, as declared in the configuration file.
 * \param[in] execution_paths List of execution paths declared in the configuration file
 * \param[out] modules Sorted list of modules, mapped to execution paths.
 *
 * \sa momemta::ModuleRegistry::exportList()
 */

void sort_modules(
        const momemta::ModuleList& available_modules,
        const std::vector<Configuration::ModuleDecl>& requested_modules,
        const std::vector<std::shared_ptr<ExecutionPath>>& execution_paths,
        SortedModuleList& modules,
        const std::string& output = ""
);

}
