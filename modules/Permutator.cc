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

#include <algorithm>
#include <cmath>

#include <TMath.h>

#include <momemta/ParameterSet.h>
#include <momemta/Module.h>
#include <momemta/Types.h>

class Permutator: public Module {
    public:

        Permutator(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()),
            m_ps_point(parameters.get<InputTag>("ps_point")),
            m_input(parameters.get<std::vector<InputTag>>("inputs"))
        {
            
            m_ps_point.resolve(pool);
            for (auto& t: m_input)
                t.resolve(pool);

            std::vector<uint32_t> tmp;
            for(uint32_t i = 0; i < m_input.size(); i++)
                tmp.push_back(i);
            do {
                perm_indices.push_back(tmp);
            } while( std::next_permutation(tmp.begin(), tmp.end()) );

            (*m_output).resize(m_input.size());
        };

        virtual void work() override {
            double psPoint = m_ps_point.get<double>();
            
            uint32_t chosen_perm = std::lround(psPoint*(perm_indices.size()-1));
            
            for(uint32_t i = 0; i < m_input.size(); i++)
                (*m_output)[i] = m_input[ perm_indices[chosen_perm][i] ].get<LorentzVector>();
        }

        virtual size_t dimensions() const override {
            return 1;
        }

    private:
        InputTag m_ps_point;
        std::vector<InputTag> m_input;

        std::shared_ptr<std::vector<LorentzVector>> m_output = produce<std::vector<LorentzVector>>("output");

        std::vector<std::vector<uint32_t>> perm_indices;
};
REGISTER_MODULE(Permutator);

