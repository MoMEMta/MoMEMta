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

/** \brief Generate a random point uniformly in a user-set range.
 *
 * Generate a random point uniformly distributed in \f$[a,b]\f$, using a phase-space point generated by Cuba (which lies in \f$[0,1]\f$).
 *
 *  - Integratio dimension: 1
 *
 *  - Parameters:
 *    - `min` (double): Start of the range
 *    - `max` (double): End of the range
 *
 *  - Inputs:
 *    - `ps_point` (double): Phase-space point generated by CUBA.
 *
 *  - Outputs:
 *    - `output` (double): Random point in [min, max].
 *    - `jacobian` (double): Jacobian of the linear transformation.
 */

#include <momemta/ParameterSet.h>
#include <momemta/Module.h>

class UniformGenerator: public Module {
    public:

        UniformGenerator(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()),
            m_min(parameters.get<double>("min")),
            m_max(parameters.get<double>("max")),
            m_ps_point(parameters.get<InputTag>("ps_point")) {
            m_ps_point.resolve(pool);
        };

        virtual void work() override {
            double psPoint = m_ps_point.get<double>();
            *output = m_min + (m_max - m_min) * psPoint;
            *jacobian = m_max - m_min; 
        }

        virtual size_t dimensions() const override {
            return 1;
        }

    private:
        const double m_min, m_max;
        InputTag m_ps_point;

        std::shared_ptr<double> output = produce<double>("output");
        std::shared_ptr<double> jacobian = produce<double>("jacobian");


};
REGISTER_MODULE(UniformGenerator);
