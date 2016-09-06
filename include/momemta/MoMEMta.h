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

#include <momemta/Module.h>
#include <momemta/ParameterSet.h>
#include <momemta/Pool.h>
#include <momemta/Types.h>

class Configuration;
class SharedLibrary;

/**
 * \brief A %MoMEMta instance
 *
 * Compute weights for a particular event using the Matrix Element Method. See [the documentation](http://momemta.github.io/introduction/) for more details about this method.
 *
 */
class MoMEMta {
    public:
        /// Status of the integration
        enum class IntegrationStatus {
            SUCCESS, ///< Integration was successful
            ACCURARY_NOT_REACHED, ///< Integration was stopped before desired accuracy was reached
            FAILED, ///< Integration failed
            ABORTED, ///< Integration aborted
            DIM_OUT_OF_RANGE, ///< Dimensions out of range
            NONE ///< No integration was performed
        };

        /** \brief Create a new MoMEMta instance
         *
         * \param configuration A frozen snapshot of the configuration, usually obtained by ConfigurationReader::freeze
         *
         * \note A single instance of MoMEMta is able to compute weights for any numbers of events. However, if you want to change the configuration, you need to create a new instance.
         */
        MoMEMta(const Configuration& configuration);
        /// Destructor
        virtual ~MoMEMta();

        /** \brief Compute the weights in the current configuration.
         *
         * This function is traditionally called from the main event loop.
         *
         * \param particles List of LorentzVector representing the final state particles. The order is important, because a mapping is done between these particles and the matrix element inside the configuration file.
         * \param met Missing transverse energy of the event. This parameter is optional.
         *
         * \return A vector of weights. Each weight is represented by a pair of double, the first element being the value of the weight and the second the associated absolute error.
         */
        std::vector<std::pair<double, double>> computeWeights(const std::vector<LorentzVector>& particles, const LorentzVector& met = LorentzVector());

        /** \brief Return the status of the integration
         *
         * \return The status of the integration
         */
        IntegrationStatus getIntegrationStatus() const;

        /**
         * \brief Read-only access to the global memory pool
         *
         * Use the pool to retrieve outputs from special modules, like DMEM.
         *
         * \return A read-only instance of the global memory pool
         */
        const Pool& getPool() const;

    private:
        class integrands_output_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        int integrand(const double* psPoints, const double* weights, double* results);

        static int CUBAIntegrand(const int *nDim, const double* psPoint, const int *nComp, double *value, void *inputs, const int *nVec, const int *core, const double *weight);

        PoolPtr m_pool;
        std::vector<ModulePtr> m_modules;

        using SharedLibraryPtr = std::shared_ptr<SharedLibrary>;
        std::vector<SharedLibraryPtr> m_libraries;

        std::size_t m_n_dimensions;
        std::size_t m_n_components;
        ParameterSet m_cuba_configuration;

        IntegrationStatus integration_status = IntegrationStatus::NONE;

        // Pool inputs
        std::shared_ptr<std::vector<double>> m_ps_points;
        std::shared_ptr<double> m_ps_weight;
        std::shared_ptr<std::vector<LorentzVector>> m_particles;
        std::shared_ptr<LorentzVector> m_met;
        std::vector<Value<double>> m_integrands;
};
