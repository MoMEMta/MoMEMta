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

#include <momemta/Module.h>

#include <vector>

#include <momemta/ParameterSet.h>
#include <momemta/Path.h>
#include <momemta/Solution.h>

#define CALL(X) { for (auto& m: path.modules()) \
        m->X(); \
    }

/**
 * \brief A module looping over a set of solutions
 *
 * For each solution, the modules of the path are executed. At least one module
 * of the path must use the `solution` output produced by this module.
 *
 * ### Integration dimension
 *
 * This module requires **0** phase-space point.
 *
 * ### Parameters
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `path` | Path | An execution path. For each solution, each module of the path will be executed in order. |
 *
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `solutions` | vector(Solution) | The set of solutions to iterate over. It's usually the output of one of the blocks.|
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `solution` | Solution | The current solution. This output only makes sense for a module inside the execution path. For any other module, this output is invalid. |
 *
 * \ingroup modules
 */
class Looper: public Module {
    public:

        Looper(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            solutions = pool->get<SolutionCollection>(parameters.get<InputTag>("solutions")); 

            path = parameters.get<Path>("path");
        };

        virtual void finish() override {
            CALL(finish);
        }

        virtual Status work() override {
            CALL(beginLoop);

            auto status = Status::OK;

            // For each solution, loop over all the modules
            for (const auto& s: *solutions) {
                *solution = s;

                for (auto& m: path.modules()) {
                    auto module_status = m->work();

                    if (module_status == Status::OK)
                        continue;
                    else if (module_status == Status::NEXT)
                        break;
                    else {
                        status = module_status;
                        break;
                    }
                }

                if (status != Status::OK)
                    break;
            }

            CALL(endLoop);

            return status;
        }

    private:
        Path path;

        // Inputs
        Value<SolutionCollection> solutions;

        // Outputs
        std::shared_ptr<Solution> solution = produce<Solution>("solution");

};
REGISTER_MODULE(Looper);
