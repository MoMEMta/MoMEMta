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


#include <momemta/ConfigurationSet.h>
#include <momemta/Module.h>
#include <momemta/Types.h>

#include <logging.h>

class Boost: public Module {
    public:

        Boost(PoolPtr pool, const ConfigurationSet& parameters): Module(pool, parameters.getModuleName()) {
            
            std::vector<InputTag> invisibles_tag = parameters.get<std::vector<InputTag>>("invisibles");
            for (const auto& tag: invisibles_tag) {
                invisibles.push_back(get<std::vector<std::vector<LorentzVector>>>(tag));
            }

            input_particles = parameters.get<std::vector<InputTag>>("particles");
            for (auto& t: input_particles)
                t.resolve(pool);
        };

        virtual void work() override {

            output->clear();

            std::vector<LorentzVector> particles;
            for (auto& p: input_particles) {
                particles.push_back(p.get<LorentzVector>());
            }

            if (!invisibles.empty()) {

                size_t n_sols = invisibles.front()->size();
                for (const auto& i: invisibles) {
                    if (i->size() != n_sols) {
                        throw std::runtime_error("All invisibles must have the same number of solutions.");
                    }
                }

                for (size_t sol_index = 0; sol_index < n_sols; sol_index++) {

                    std::vector<LorentzVector> particles_and_invisibles = particles;

                    for (const auto& i: invisibles) {

                        for (const auto& p: (*i)[sol_index])
                            particles_and_invisibles.push_back(p);
                    
                    }
                
                    compute_boost(particles_and_invisibles);
                }
                
            } else {
                compute_boost(particles);
            }
        }

        void compute_boost(const std::vector<LorentzVector>& particles) {
            LorentzVector tot;
            for (const auto& p: particles)
                tot += p;

            double q1Pz = (tot.Pz() + tot.E()) / 2.;
            double q2Pz = (tot.Pz() - tot.E()) / 2.;

            std::vector<LorentzVector> partons {
                LorentzVector(0., 0., q1Pz, std::abs(q1Pz)),
                LorentzVector(0., 0., q2Pz, std::abs(q2Pz))
            };

            output->push_back(partons);
        }

    private:
        std::vector<std::shared_ptr<const std::vector<std::vector<LorentzVector>>>> invisibles;
        std::vector<InputTag> input_particles;

        std::shared_ptr<std::vector<std::vector<LorentzVector>>> output = produce<std::vector<std::vector<LorentzVector>>>("output");
};
REGISTER_MODULE(Boost);
