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

#include <momemta/ParameterSet.h>
#include <momemta/Module.h>
#include <momemta/Types.h>
#include <momemta/Utils.h>

#include <stdexcept>

/** \brief Performs linear combination of four-vectors.
 *
 * This module returns a linear combination of four-vectors e.g. if it is called with `(v1, v2, v3, ...)` and `(a, b, c, ...)` it will return `a*v1 + b*v2 + c*v3 + ...`.
 *
 * ### Integration dimension
 *
 * This module requires **0** phase-space point.
 *
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `inputs` | vector(LorentzVector) | List of LorentzVector used in the linear operation. |
 *   | `coeficients` | vector(double) | List of the coeficients multiplying each LorentzVector in the linear combination (must have same size as `inputs`). |
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `output` | LorentzVector | LorentzVector resulting form the linear combination. |
 *
 * \ingroup modules
 */

class VectorLinearCombinator: public Module {
    public:

        VectorLinearCombinator(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            auto vectors_tags = parameters.get<std::vector<InputTag>>("inputs");
            for (auto& v: vectors_tags)
                m_vectors.push_back(get<LorentzVector>(v));

            m_coeficients = parameters.get<std::vector<double>>("coeficients");
        };

        virtual Status work() override {

            if (m_coeficients.size() != m_vectors.size()){
                auto excepion = std::invalid_argument("VectorLinearCombinator recieved LorentzVector and Coeficient listis having different size.");
                LOG(fatal) << excepion.what();
                throw excepion;
            }

            LorentzVector temp_result(0,0,0,0);
            for (std::size_t i = 0; i < m_vectors.size(); i++)
                temp_result += m_coeficients[i] * *m_vectors[i];

            *output = temp_result;

            return Status::OK;
        }

    private:

        // Inputs
        std::vector<double> m_coeficients;
        std::vector<Value<LorentzVector>> m_vectors;

        // Outputs
        std::shared_ptr<LorentzVector> output = produce<LorentzVector>("output");
};
REGISTER_MODULE(VectorLinearCombinator);
