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

/** \brief Performs linear combination of templated terms.
 *
 * This module returns a linear combination of objects e.g. if it is called with `(v1, v2, v3, ...)` and `(a, b, c, ...)` it will return `a*v1 + b*v2 + c*v3 + ...`.
 *
 * ### Integration dimension
 *
 * This module requires **0** phase-space point.
 *
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `inputs` | vector(T) | List of terms used in the linear operation. |
 *   | `coefficients` | vector(double) | List of the coefficients multiplying each term in the linear combination (must have same size as `inputs`). |
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `output` | T | Object resulting form the linear combination. |
 *
 * \ingroup modules
 */

template<typename T>
class LinearCombinator: public Module {
    public:

        LinearCombinator(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            auto tags = parameters.get<std::vector<InputTag>>("inputs");
            for (auto& v: tags)
                m_terms.push_back(get<T>(v));

            m_coefficients = parameters.get<std::vector<double>>("coefficients");
        };

        virtual Status work() override {

            if (m_coefficients.size() != m_terms.size()){
                auto exception = std::invalid_argument("The Term and Coefficient lists passed to LinearCombinator have different sizes.");
                LOG(fatal) << exception.what();
                throw exception;
            }

            T temp_result = m_coefficients[0] * *m_terms[0];
            for (std::size_t i = 1; i < m_terms.size(); i++)
                temp_result += m_coefficients[i] * *m_terms[i];

            *output = temp_result;

            return Status::OK;
        }

    private:

        // Inputs
        std::vector<double> m_coefficients;
        std::vector<Value<T>> m_terms;

        // Outputs
        std::shared_ptr<T> output = produce<T>("output");
};

REGISTER_MODULE_NAME("VectorLinearCombinator", LinearCombinator<LorentzVector>);
REGISTER_MODULE_NAME("DoubleLinearCombinator", LinearCombinator<double>);
REGISTER_MODULE_NAME("IntLinearCombinator", LinearCombinator<int64_t>);
