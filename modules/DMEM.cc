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

/*
 * \brief Module implementing the Differential MEM
 *
 * \ingroup modules
 */

class DMEM: public Module {
    public:

        DMEM(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName())
        {
            x_start = parameters.get<double>("x_start");
            x_end = parameters.get<double>("x_end");
            n_bins = parameters.get<int64_t>("n_bins");

            m_hist = produce<TH1D>("hist", (name() + "_DMEM").c_str(), (name() + "_DMEM").c_str(), n_bins, x_start, x_end);
            m_hist->SetDirectory(0);

            m_particle_tags = parameters.get<std::vector<InputTag>>("particles");
            for (auto& t: m_particle_tags)
              t.resolve(pool);
            
            InputTag invisibles_tag = parameters.get<InputTag>("invisibles");
            m_invisibles = get<std::vector<std::vector<LorentzVector>>>(invisibles_tag);

            psWeight = get<double>(parameters.get<InputTag>("ps_weight"));
            integrands = get<std::vector<double>>(parameters.get<InputTag>("integrands"));
        }

        virtual void beginIntegration() override {
            m_hist->Reset();
        }

        virtual void work() override {
            
            if (integrands->size() == 0)
                return;

            LorentzVector tot;
            for(const auto &v: m_particle_tags)
                tot += v.get<LorentzVector>();

            // No invisibles -> loop over particles we got as input
            if(!m_invisibles.get() || m_invisibles->empty()) {
                
                m_hist->Fill(tot.M(), (*integrands)[0] * (*psWeight));
            
            } else {
        
                // Several solutions for the invisibles -> loop over all solution,
                // fill histogram with corresponding integrand.
                for(size_t sol = 0; sol < m_invisibles->size(); sol++) {
                    LorentzVector thisTot = tot;
                    
                    for(const auto &v: (*m_invisibles)[sol])
                      thisTot += v;

                      m_hist->Fill(thisTot.M(), (*integrands)[sol] * (*psWeight));
                }

            }
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
        std::shared_ptr<const std::vector<std::vector<LorentzVector>>> m_invisibles;
        
        std::shared_ptr<const double> psWeight;
        std::shared_ptr<const std::vector<double>> integrands;
        
        std::shared_ptr<TH1D> m_hist;
};
REGISTER_MODULE(DMEM);
