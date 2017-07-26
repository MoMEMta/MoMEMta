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

#include <momemta/config.h>
#include <momemta/Module.h>
#include <momemta/ParameterSet.h>
#include <momemta/Particle.h>
#include <momemta/Pool.h>
#include <momemta/Types.h>

class Configuration;
class SharedLibrary;

#ifdef DEBUG_TIMING
#include <chrono>
#endif

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
            ACCURACY_NOT_REACHED, ///< Integration was stopped before desired accuracy was reached
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
         * \param particles List of Particle representing the final state particles.
         * \param met Missing transverse energy of the event. This parameter is optional.
         *
         * \return A vector of weights. Each weight is represented by a pair of double, the first element being the value of the weight and the second the associated absolute error.
         */
        std::vector<std::pair<double, double>> computeWeights(const std::vector<momemta::Particle>& particles,
                                                              const LorentzVector& met = LorentzVector());

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
        class cuba_configuration_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };
        class unphysical_lorentzvector_error: public std::exception {
        public:
            unphysical_lorentzvector_error(const LorentzVector& p4);
            virtual const char* what() const noexcept override;
        private:
            std::string _what;
        };
        class invalid_inputs: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };
        class integrands_nonfinite_error: public std::runtime_error {
            using std::runtime_error::runtime_error;
        };

        /**
         * \brief Test if a LorentzVector is physical or not
         *
         * To be considered as physical, the energy and the mass of the lorentz vector must be positive
         *
         * \param p4 The lorentz vector to test
         */
        void checkIfPhysical(const LorentzVector& p4);

        int integrand(const double* psPoints, double* results, const double* weights);

        static int CUBAIntegrand(const int *nDim, const double* psPoint, const int *nComp, double *value, void *inputs, const int *nVec, const int *core);
        static int CUBAIntegrandWeighted(const int *nDim, const double* psPoint, const int *nComp, double *value, void *inputs, const int *nVec, const int *core, const double *weight);
        static void cuba_logging(const char*);

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

        std::unordered_map<std::string, std::shared_ptr<LorentzVector>> m_inputs_p4;
        std::unordered_map<std::string, std::shared_ptr<int64_t>> m_inputs_type;
        std::shared_ptr<LorentzVector> m_met;
        std::vector<Value<double>> m_integrands;

#ifdef DEBUG_TIMING
        std::unordered_map<Module*, std::chrono::high_resolution_clock::duration> m_module_timing;
#endif
};
