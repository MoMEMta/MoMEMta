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

#include <sstream>

#include <momemta/ParameterSet.h>
#include <momemta/Solution.h>
#include <momemta/Types.h>

/**
 * \brief A module printing the value of an input
 */
template<typename T>
class Printer: public Module {
    public:

        Printer(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            auto tag = parameters.get<InputTag>("input");
            input = pool->get<T>(tag); 
            name = parameters.get<std::string>("name", tag.toString());
        };

        virtual Status work() override {
            LOG(info) << name << ": " << *input;

            return Status::OK;
        }

        virtual bool leafModule() const override {
            return true;
        }

    private:
        std::string name;

        // Inputs
        Value<T> input;
};

/**
 * \brief Specialization of Printer for `std::vector`
 */
template<typename T>
class Printer<std::vector<T>>: public Module {
    public:

        Printer(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            auto tag = parameters.get<InputTag>("input");
            input = pool->get<std::vector<T>>(tag); 
            name = parameters.get<std::string>("name", tag.toString());
        };

        virtual Status work() override {
            std::stringstream str;
            str << name << ": {";

            size_t count = 1;
            for (const auto& i: *input) {
                str << i;
                if (count < input->size())
                    str << ", ";
                count++;
            }
            LOG(info) << str.str() << "}";

            return Status::OK;
        }

        virtual bool leafModule() const override {
            return true;
        }

    private:
        std::string name;

        // Inputs
        Value<std::vector<T>> input;
};

REGISTER_MODULE_NAME("IntPrinter", Printer<int64_t>);
REGISTER_MODULE_NAME("DoublePrinter", Printer<double>);
REGISTER_MODULE_NAME("P4Printer", Printer<LorentzVector>);
REGISTER_MODULE_NAME("SolutionPrinter", Printer<Solution>);

REGISTER_MODULE_NAME("IntVectorPrinter", Printer<std::vector<int64_t>>);
REGISTER_MODULE_NAME("DoubleVectorPrinter", Printer<std::vector<double>>);
REGISTER_MODULE_NAME("P4VectorPrinter", Printer<std::vector<LorentzVector>>);
REGISTER_MODULE_NAME("SolutionVectorPrinter", Printer<SolutionCollection>);
