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


#pragma once

#include <memory>
#include <vector>

#include <logging.h>
#include <momemta/ConfigurationReader.h>
#include <momemta/Module.h>
#include <momemta/Pool.h>
#include <momemta/Types.h>

class SharedLibrary;

class MoMEMta {
    public:
        MoMEMta(const ConfigurationReader&);
        virtual ~MoMEMta();

        std::vector<std::pair<double, double>> computeWeights(const std::vector<LorentzVector>& particules);

        double integrand(const double* psPoints, const double* weights);


    private:
        static int CUBAIntegrand(const int *nDim, const double* psPoint, const int *nComp, double *value, void *inputs, const int *nVec, const int *core, const double *weight);

        PoolPtr m_pool;
        std::vector<ModulePtr> m_modules;

        using SharedLibraryPtr = std::shared_ptr<SharedLibrary>;
        std::vector<SharedLibraryPtr> m_libraries;

        size_t m_n_dimensions;
        std::shared_ptr<ConfigurationSet> m_vegas_configuration;

        // Pool inputs
        std::shared_ptr<std::vector<double>> m_ps_points;
        std::shared_ptr<std::vector<LorentzVector>> m_particles;
        std::shared_ptr<const std::vector<double>> m_weights;
};
