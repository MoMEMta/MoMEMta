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

#include <functional>

#include <Math/Vector3D.h>
#include <Math/Boost.h>

#include <momemta/Module.h>
#include <momemta/ParameterSet.h>
#include <momemta/Types.h>

/** \brief Build the initial partons given the whole final state
 *
 * Define the initial partons 4-momenta given the particles present in the final state.
 *
 * If the parameter `do_transverse_boost` is set to `true`, the partons are boosted in the transverse direction
 * to satisfy energy-momentum conservation. 
 * Otherwise, the partons are simply built using the total energy and longitudinal momentum, without enforcing
 * conservation of transverse momentum.
 *
 * Use of the transverse boost or not can depend on how the invisibles are reconstructed by the blocks: 
 * if the block already enforces momentum conservation, applying the boost or not will yield the same result.
 *
 * ### Integration dimension
 *
 * This module requires **0** phase-space point.
 *
 * ### Parameters
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `do_transverse_boost` | bool, default: `false` | Boost the initial partons in the transverse direction to match total transverse momentum. | 
 *
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `particles` | vector(LorentzVector) | Set of all particles. |
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `partons` | vector(LorentzVector) | Sets of initial parton 4-momenta. | 
 *
 * \ingroup modules
 */
class BuildInitialState: public Module {
    public:

        BuildInitialState(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            if (parameters.get<bool>("do_transverse_boost", false)) {
                do_compute_initials = compute_initials_boost;
            } else {
                do_compute_initials = compute_initials_trivial;
            }

            std::vector<InputTag> input_particles_tags = parameters.get<std::vector<InputTag>>("particles");
            for (auto& t: input_particles_tags)
                input_particles.push_back(get<LorentzVector>(t));
        };

        virtual Status work() override {

            partons->clear();

            LorentzVectorRefCollection particles;
            for (auto& p: input_particles) {
                particles.push_back(std::ref(*p));
            }

            do_compute_initials(particles);

            return Status::OK;
        }

    private:
        using compute_initials_signature = std::function<void(const LorentzVectorRefCollection&)>;

        compute_initials_signature do_compute_initials;

        compute_initials_signature compute_initials_trivial =
            [&](const LorentzVectorRefCollection& particles) {
                LorentzVector tot;
                for (const auto& p: particles)
                    tot += p.get();

                double q1Pz = (tot.Pz() + tot.E()) / 2.;
                double q2Pz = (tot.Pz() - tot.E()) / 2.;

                partons->push_back(LorentzVector(0., 0., q1Pz, std::abs(q1Pz)));
                partons->push_back(LorentzVector(0., 0., q2Pz, std::abs(q2Pz)));
            };

        compute_initials_signature compute_initials_boost =
            [&](const LorentzVectorRefCollection& particles) {
                LorentzVector tot;
                for (const auto& p: particles)
                    tot += p.get();
                
                // Define boost that puts the transverse total momentum vector in its CoM frame 
                LorentzVector transverse_tot = tot;
                transverse_tot.SetPz(0);
                ROOT::Math::XYZVector isr_deBoost_vector( transverse_tot.BoostToCM() );

                // In the "transverse" CoM frame, use total Pz and E to define initial longitudinal quark momenta
                const ROOT::Math::Boost isr_deBoost( isr_deBoost_vector );
                const ROOT::Math::PxPyPzEVector new_tot( isr_deBoost * tot );

                double q1Pz = (new_tot.Pz() + new_tot.E()) / 2.;
                double q2Pz = (new_tot.Pz() - new_tot.E()) / 2.;

                partons->push_back(LorentzVector(0., 0., q1Pz, std::abs(q1Pz)));
                partons->push_back(LorentzVector(0., 0., q2Pz, std::abs(q2Pz)));

                // Boost initial parton momenta by the opposite of the transverse boost needed to put the whole system in its CoM
                const ROOT::Math::Boost isr_boost( -isr_deBoost_vector );
                (*partons)[0] = isr_boost * (*partons)[0];
                (*partons)[1] = isr_boost * (*partons)[1];
            };

        std::vector<Value<LorentzVector>> input_particles;

        std::shared_ptr<std::vector<LorentzVector>> partons = produce<std::vector<LorentzVector>>("partons");
};
REGISTER_MODULE(BuildInitialState);
