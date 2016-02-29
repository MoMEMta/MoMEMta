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

class Flatter: public Module {
    public:

        Flatter(PoolPtr pool, const ConfigurationSet& parameters): Module(pool, parameters.getModuleName()),
            mass(parameters.get<double>("mass")),
            width(parameters.get<double>("width")),
            m_ps_point(parameters.get<InputTag>("input")) {
            m_ps_point.resolve(pool);
        };

        virtual void work() override {

            double psPoint = m_ps_point.get<double>();
            const double range = M_PI / 2. + std::atan(mass / width);
            const double y = - std::atan(mass / width) + range * psPoint;

            *s = mass * width * std::tan(y) + (mass * mass);
            *jacobian = range * mass * width / (std::cos(y) * std::cos(y));
        }

        virtual size_t dimensions() const override {
            return 1;
        }

    private:
        const float mass;
        const float width;
        InputTag m_ps_point;

        std::shared_ptr<double> s = produce<double>("s");
        std::shared_ptr<double> jacobian = produce<double>("jacobian");


};
REGISTER_MODULE(Flatter);
