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
 * A loop is done over the set of solution. For each solution, the outputs are filled with the
 * list of particles and the jacobian, and the modules of the path are executed. At least one module
 * must use the output produced by this module.
 *
 * If the path is executed, the solution is ensured to be valid.
 *
 * ### In details
 *
 * Each blocks produce a set of solutions. In order to define the final integrand value, various computations
 * (matrix element, initial state, …) must be performed on **each** solution. Letting each module handling multiple
 * solutions, but it leads to many problems, the biggest one being the synchronization between modules: a solution
 * can be seen as valid from a module, but not for another. Instead, each module only handles a set of particle,
 * without the necessity to know where it comes from (input particle or solution of a block), and the iteration
 * over the solutions is delegated to this Looper module. For each solution, a sequence of module is executed,
 * as described by the `path` argument.
 *
 * Schematically, things can be represented by this graph:
 *
 * ```
 *  ───────
 *   Block
 *  ───┬───
 *     │
 *     ├─ Solutions
 *     │
 *  ───┴────
 *   Looper
 *  ─┬────┬─
 *   │    │
 *   │    ├─ Solution
 *   │    │
 *   │  ──┴─────
 *   │   Module
 *   │  ──┬─────
 *   │    │
 *   │    │
 *   │  ──┴─────
 *   │   Module
 *   │  ──┬─────
 *   │    │
 *   └─┬──┘
 *     │
 *  ───┴───
 *   Module
 *  ───┬───
 * ```
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
 *   | `particles` | vector(LorentzVector) | The particles of the current solution. This output only makes sense for a module inside the execution path. For any other module, this output is invalid. |
 *   | `jacobian` | double | The jacobian of the current solution. This output only makes sense for a module inside the execution path. For any other module, this output is invalid. |
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
            particles->clear();

            CALL(beginLoop);

            auto status = Status::OK;

            // For each solution, loop over all the modules
            for (const auto& s: *solutions) {
                if (!s.valid)
                    continue;

                *particles = s.values;
                *jacobian = s.jacobian;

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
        std::shared_ptr<std::vector<LorentzVector>> particles = produce<std::vector<LorentzVector>>("particles");
        std::shared_ptr<double> jacobian = produce<double>("jacobian");

};
REGISTER_MODULE(Looper);
