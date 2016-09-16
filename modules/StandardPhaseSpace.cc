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

#include <momemta/Math.h>
#include <momemta/Module.h>
#include <momemta/ParameterSet.h>
#include <momemta/Types.h>

/** \brief Compute the phase space density for observed particles (not concerned by the change of variable)
 *
 * \f[
 *  d\Phi = \prod_{i \in \text{vis.}} \frac{\left|p_i\right|^2 \sin(\theta_i)}{2 E_i(2\pi)^3}
 * \f]
 *
 *
 * ### Integration dimension
 *
 * This module requires **0** phase-space point.
 *
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `particles` | vector(LorentzVector) | Set of all observed particles (not concerned by the change of variable). |
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `phase_space` | double | Phase space density. |
 *
 * \ingroup modules
 */
class StandardPhaseSpace: public Module {
    public:

        StandardPhaseSpace(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            std::vector<InputTag> input_particles_tags = parameters.get<std::vector<InputTag>>("particles");
            for (auto& t: input_particles_tags)
                input_particles.push_back(get<LorentzVector>(t));
        };

        virtual Status work() override {

            *phase_space = 1;
            for (const auto& p: input_particles) {
                *phase_space *= SQ(p->P()) * sin(p->Theta()) / (2.0 * p->E() * CB(2. * M_PI));
            }

            return Status::OK;
        }

    private:

        // Inputs
        std::vector<Value<LorentzVector>> input_particles;

        // Outputs
        std::shared_ptr<double> phase_space = produce<double>("phase_space");
};
REGISTER_MODULE(StandardPhaseSpace);
