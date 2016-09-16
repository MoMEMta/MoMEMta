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

#include <momemta/MoMEMta.h>

#include <cstring>

#include <cuba.h>

#include <momemta/Configuration.h>
#include <momemta/Logging.h>
#include <momemta/ParameterSet.h>
#include <momemta/Utils.h>
#include <momemta/Unused.h>

#include <Graph.h>

MoMEMta::MoMEMta(const Configuration& configuration) {

    // Initialize shared memory pool for modules
    m_pool.reset(new Pool());

    // Create phase-space points vector, input for many modules
    m_pool->current_module("cuba");
    m_ps_points = m_pool->put<std::vector<double>>({"cuba", "ps_points"});
    m_ps_weight = m_pool->put<double>({"cuba", "ps_weight"});

    // Create vector for input particles
    m_pool->current_module("input");
    m_particles = m_pool->put<std::vector<LorentzVector>>({"input", "particles"});
    // Create input for met
    m_met = m_pool->put<LorentzVector>({"input", "met"});

    // Construct modules from configuration
    std::vector<Configuration::Module> light_modules = configuration.getModules();
    for (const auto& module: light_modules) {
        m_pool->current_module(module);
        m_modules.push_back(ModuleFactory::get().create(module.type, m_pool, *module.parameters));
        m_modules.back()->configure();
    }

    m_n_dimensions = configuration.getNDimensions();
    LOG(info) << "Number of dimensions for integration: " << m_n_dimensions;

    // Resize pool ps-points vector
    m_ps_points->resize(m_n_dimensions);

    // Integrand
    // First, check if the user defined which integrand to use
    std::vector<InputTag> integrands = configuration.getIntegrands();
    if (!integrands.size()) {
        LOG(fatal) << "No integrand found. Define which module's output you want to use as the integrand using the lua `integrand` function.";
        throw integrands_output_error("No integrand found");
    }

    // Next, retrieve all the input tags for the components of the integrand
    m_pool->current_module("momemta");

    for(const auto& component: integrands) {
        m_integrands.push_back(m_pool->get<double>(component));
        LOG(debug) << "Configuration declared integrand component using: " << component.toString();
    }
    m_n_components = m_integrands.size();

    m_cuba_configuration = configuration.getCubaConfiguration();

    const Pool::DescriptionMap& description = m_pool->description();
    graph::build(description, m_modules, configuration.getPaths(), [&description, this](const std::string& module) {
                // Clean the pool for each removed module
                const Description& d = description.at(module);
                for (const auto& input: d.inputs)
                    this->m_pool->remove_if_invalid(input);
                for (const auto& output: d.outputs)
                    this->m_pool->remove({module, output});
            });

    // Freeze the pool after removing unneeded modules
    m_pool->freeze();

    cubacores(0, 0);
}

MoMEMta::~MoMEMta() {
    for (const auto& module: m_modules) {
        module->finish();
    }
}

const Pool& MoMEMta::getPool() const {
    return *m_pool;
}

std::vector<std::pair<double, double>> MoMEMta::computeWeights(const std::vector<LorentzVector>& particles, const LorentzVector& met) {
    
    for (const auto& module: m_modules) {
        module->beginIntegration();
    }

    *m_particles = particles;
    *m_met = met;

    // Read cuba configuration
    uint8_t verbosity = m_cuba_configuration.get<int64_t>("verbosity", 0);
    bool subregion = m_cuba_configuration.get<bool>("subregion", false);
    bool smoothing = m_cuba_configuration.get<bool>("smoothing", true);
    bool retainStateFile = m_cuba_configuration.get<bool>("retainStateFile", false);
    bool takeOnlyGridFromFile = m_cuba_configuration.get<bool>("takeOnlyGridFromFile", true);
    uint64_t level = m_cuba_configuration.get<int64_t>("level", 0);

    double relative_accuracy = m_cuba_configuration.get<double>("relative_accuracy", 0.005);
    double absolute_accuracy = m_cuba_configuration.get<double>("absolute_accuracy", 0.);
    int64_t seed = m_cuba_configuration.get<int64_t>("seed", 0);
    int64_t min_eval = m_cuba_configuration.get<int64_t>("min_eval", 0);
    int64_t max_eval = m_cuba_configuration.get<int64_t>("max_eval", 500000);
    int64_t n_start = m_cuba_configuration.get<int64_t>("n_start", 25000);
    int64_t n_increase = m_cuba_configuration.get<int64_t>("n_increase", 0);
    int64_t batch_size = m_cuba_configuration.get<int64_t>("batch_size", 12500);

    unsigned int flags = cuba::createFlagsBitset(verbosity, subregion, retainStateFile, level, smoothing, takeOnlyGridFromFile);

    std::string grid_file = m_cuba_configuration.get<std::string>("grid_file", "");
    int64_t grid_number = m_cuba_configuration.get<int64_t>("grid_number", 0);
    
    // Output from cuba
    long long int neval = 0;
    int nfail = 0;
    std::unique_ptr<double[]> mcResult(new double[m_n_components]); 
    std::unique_ptr<double[]> prob(new double[m_n_components]); 
    std::unique_ptr<double[]> error(new double[m_n_components]); 
    
    for (size_t i = 0; i < m_n_components; i++) {
        mcResult[i] = 0;
        prob[i] = 0;
        error[i] = 0;
    }
    
    llVegas(
         m_n_dimensions,         // (int) dimensions of the integrated volume
         m_n_components,         // (int) dimensions of the integrand
         (integrand_t) CUBAIntegrand,  // (integrand_t) integrand (cast to integrand_t)
         (void*) this,           // (void*) pointer to additional arguments passed to integrand
         1,                      // (int) maximum number of points given the integrand in each invocation (=> SIMD) ==> PS points = vector of sets of points (x[ndim][nvec]), integrand returns vector of vector values (f[ncomp][nvec])
         relative_accuracy,      // (double) requested relative accuracy  /
         absolute_accuracy,      // (double) requested absolute accuracy /-> error < max(rel*value,abs)
         flags,                  // (int) various control flags in binary format, see setFlags function
         seed,                   // (int) seed (seed==0 => SOBOL; seed!=0 && control flag "level"==0 => Mersenne Twister)
         min_eval,               // (int) minimum number of integrand evaluations
         max_eval,               // (int) maximum number of integrand evaluations (approx.!)
         n_start,                // (int) number of integrand evaluations per interations (to start)
         n_increase,             // (int) increase in number of integrand evaluations per interations
         batch_size,             // (int) batch size for sampling
         grid_number,            // (int) grid number, 1-10 => up to 10 grids can be stored, and re-used for other integrands (provided they are not too different)
         grid_file.c_str(),      // (char*) name of state file => state can be stored and retrieved for further refinement
         NULL,                   // (int*) "spinning cores": -1 || NULL <=> integrator takes care of starting & stopping child processes (other value => keep or retrieve child processes, probably not useful here)
         &neval,                 // (int*) actual number of evaluations done
         &nfail,                 // 0=desired accuracy was reached; -1=dimensions out of range; >0=accuracy was not reached
         mcResult.get(),         // (double*) integration result ([ncomp])
         error.get(),            // (double*) integration error ([ncomp])
         prob.get()              // (double*) Chi-square p-value that error is not reliable (ie should be <0.95) ([ncomp])
    );
    
    if (nfail == 0) {
        integration_status = IntegrationStatus::SUCCESS;
    } else if (nfail == -1) {
        integration_status = IntegrationStatus::DIM_OUT_OF_RANGE;
    } else if (nfail > 0) {
        integration_status = IntegrationStatus::ACCURARY_NOT_REACHED;
    } else if (nfail == -99) {
        integration_status = IntegrationStatus::ABORTED;
    }

    for (const auto& module: m_modules) {
        module->endIntegration();
    }

    std::vector<std::pair<double, double>> result;
    for (size_t i = 0; i < m_n_components; i++) {
        result.push_back( std::make_pair(mcResult[i], error[i]) );
    }

    return result;
}

#define CUBA_ABORT -999
#define CUBA_OK 0

int MoMEMta::integrand(const double* psPoints, const double* weights, double* results) {

    // Store phase-space points into the pool
    std::memcpy(m_ps_points->data(), psPoints, sizeof(double) * m_n_dimensions);

    // Store phase-space weight into the pool
    *m_ps_weight = *weights;

    for (auto& module: m_modules) {
        auto status = module->work();

        if (status == Module::Status::NEXT) {
            // Stop executation for the current integration step
            // Returns 0 so that cuba knows this phase-space volume is not relevant
            for (size_t i = 0; i < m_n_components; i++)
                results[i] = 0;
            return CUBA_OK;
        } else if (status == Module::Status::ABORT) {
            // Abort integration
            for (size_t i = 0; i < m_n_components; i++)
                results[i] = 0;
            return CUBA_ABORT;
        }
    }

    for (size_t i = 0; i < m_n_components; i++)
        results[i] = *(m_integrands[i]);

    return CUBA_OK;
}

int MoMEMta::CUBAIntegrand(const int *nDim, const double* psPoint, const int *nComp, double *value, void *inputs, const int *nVec, const int *core, const double *weight) {
    UNUSED(nDim);
    UNUSED(nComp);
    UNUSED(nVec);
    UNUSED(core);

    return static_cast<MoMEMta*>(inputs)->integrand(psPoint, weight, value);
}

MoMEMta::IntegrationStatus MoMEMta::getIntegrationStatus() const {
    return integration_status;
}
