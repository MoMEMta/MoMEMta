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

#include <vector>

#include <momemta/ParameterSet.h>
#include <momemta/Types.h>

/**
 * \brief A module counting the number of entries in a vector
 */
template<typename T>
class Counter: public Module {
    public:

        Counter(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            LOG(info) << "Counter constructor: " << parameters.getModuleName();
            input = pool->get<std::vector<T>>(parameters.get<InputTag>("input")); 
        };

        virtual void beginIntegration() override {
            *result = 0;
        }

        virtual void beginLoop() override {
            *result = 0;
        }

        virtual Status work() override {
            *result += input->size();

            return Status::OK;
        }

    private:

        // Inputs
        Value<std::vector<T>> input;

        // Outputs
        std::shared_ptr<int64_t> result = produce<int64_t>("count");

};

REGISTER_MODULE_NAME("IntCounter", Counter<int64_t>);
REGISTER_MODULE_NAME("DoubleCounter", Counter<double>);
REGISTER_MODULE_NAME("LorentzVectorCounter", Counter<LorentzVector>);

/**
 * \brief A simple module counting the number of time it has been called
 */
class SimpleCounter: public Module {
    public:

        SimpleCounter(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
        };

        virtual void beginIntegration() override {
            *result = 0;
        }

        virtual void beginLoop() override {
            *result = 0;
        }

        virtual Status work() override {
            *result += 1;

            return Status::OK;
        }

    private:

        // Outputs
        std::shared_ptr<int64_t> result = produce<int64_t>("count");

};

REGISTER_MODULE(SimpleCounter);
