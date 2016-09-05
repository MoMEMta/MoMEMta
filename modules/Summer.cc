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

#include <momemta/Module.h>

#include <momemta/ParameterSet.h>
#include <momemta/Types.h>

/**
 * \brief A module performing a sum over a set of values
 */
template<typename T>
class Summer: public Module {
    public:

        Summer(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            input = pool->get<T>(parameters.get<InputTag>("input")); 
        };

        virtual void beginLoop() override {
            *result = 0;
        }

        virtual Status work() override {
            *result += *input;

            return Status::OK;
        }

    private:

        // Inputs
        Value<T> input;

        // Outputs
        std::shared_ptr<T> result = produce<T>("sum");

};

/**
 * \brief Specialization for LorentzVector
 */
template<>
void Summer<LorentzVector>::beginLoop() {
  result->SetXYZT(0, 0, 0, 0);
}

REGISTER_MODULE_NAME("IntSummer", Summer<int64_t>);
REGISTER_MODULE_NAME("DoubleSummer", Summer<double>);
REGISTER_MODULE_NAME("P4Summer", Summer<LorentzVector>);
