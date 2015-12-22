#include <ConfigurationSet.h>
#include <Module.h>
#include <logging.h>
#include <Types.h>

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

                int n_sols = -1;
                for (const auto& i: invisibles) {
                    if (n_sols < 0)
                        n_sols = i->size();
                    else {
                        if (i->size() != n_sols) {
                            throw std::runtime_error("All invisibles must have the same number of solutions.");
                        }
                    }
                }

                for (size_t sol_index = 0; sol_index < (size_t) n_sols; sol_index++) {

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
                LorentzVector(0., 0., q1Pz, q1Pz),
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
