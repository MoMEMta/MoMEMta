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

/**
 * \brief A module declaring a constant
 *
 * \ingroup modules
 */
template<typename T>
class Constant: public Module {
    public:

    Constant(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            value = parameters.get<T>("value");
        };

        virtual void beginIntegration() override {
            *constant = value;
        };

    private:
        T value;

        // Outputs
        std::shared_ptr<T> constant = produce<T>("value");
};

REGISTER_MODULE_NAME("IntConstant", Constant<int64_t>)
        .Output("value")
        .Attr("value:int");

REGISTER_MODULE_NAME("DoubleConstant", Constant<double>)
        .Output("value")
        .Attr("value:double");
