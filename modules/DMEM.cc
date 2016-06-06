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

#include <TH1D.h>

#include <momemta/ParameterSet.h>
#include <momemta/Module.h>
#include <momemta/Solution.h>

/*
 * \brief Module implementing the Differential MEM
 *
 * \ingroup modules
 */

class DMEM: public Module {
    public:

        DMEM(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            x_start = parameters.get<double>("x_start");
            x_end = parameters.get<double>("x_end");
            n_bins = parameters.get<int64_t>("n_bins");

            m_hist = produce<TH1D>("hist", (name() + "_DMEM").c_str(), (name() + "_DMEM").c_str(), n_bins, x_start, x_end);
            m_hist->SetDirectory(0);

            m_particle_tags = parameters.get<std::vector<InputTag>>("particles");
            for (auto& t: m_particle_tags)
              t.resolve(pool);
            
            InputTag invisibles_tag = parameters.get<InputTag>("invisibles");
            m_solution = get<Solution>(invisibles_tag);

            psWeight = get<double>(parameters.get<InputTag>("ps_weight"));
            meOutput = get<double>(parameters.get<InputTag>("me_output"));
        }

        virtual void beginIntegration() override {
            m_hist->Reset();
        }

        virtual Status work() override {
            
            LorentzVector tot;
            for (const auto& v: m_particle_tags)
                tot += v.get<LorentzVector>();

            // Add solution particles if we have some
            for (const auto& i: m_solution->values)
                tot += i;

            // Fill histogram
            m_hist->Fill(tot.M(), *meOutput * (*psWeight));

            return Status::OK;
        }

        virtual size_t dimensions() const override {
            return 0;
        }

        virtual bool leafModule() const override {
            return true;
        }

    private:

        double x_start, x_end;
        int64_t n_bins;
        
        std::vector<InputTag> m_particle_tags;
        std::shared_ptr<const Solution> m_solution;
        
        std::shared_ptr<const double> psWeight;
        std::shared_ptr<const double> meOutput;
        
        std::shared_ptr<TH1D> m_hist;
};
REGISTER_MODULE(DMEM);
