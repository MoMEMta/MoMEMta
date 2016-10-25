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

#ifdef DEBUG_TIMING
using namespace std::chrono;
#endif

#define CUBA_ABORT -999
#define CUBA_OK 0

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
        try {
            m_modules.push_back(ModuleFactory::get().create(module.type, m_pool, *module.parameters));
            m_modules.back()->configure();
        } catch (...) {
            LOG(fatal) << "Exception while trying to create module " << module.type << "::" << module.name
                       << ". See message above for a (possible) more detailed description of the error.";
            std::rethrow_exception(std::current_exception());
        }
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

    // Register logging function
    cubalogging(MoMEMta::cuba_logging);
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

    for (const auto& p: particles)
        checkIfPhysical(p);

    checkIfPhysical(met);

    *m_particles = particles;
    *m_met = met;

    for (const auto& module: m_modules) {
        module->beginIntegration();
    }

    std::unique_ptr<double[]> mcResult(new double[m_n_components]);
    std::unique_ptr<double[]> error(new double[m_n_components]);

    for (size_t i = 0; i < m_n_components; i++) {
        mcResult[i] = 0;
        error[i] = 0;
    }

    if (m_n_dimensions > 0) {

        // Read cuba configuration

        std::string algorithm = m_cuba_configuration.get<std::string>("algorithm", "vegas");

        // Common arguments
        double relative_accuracy = m_cuba_configuration.get<double>("relative_accuracy", 0.005);
        double absolute_accuracy = m_cuba_configuration.get<double>("absolute_accuracy", 0.);
        int64_t seed = m_cuba_configuration.get<int64_t>("seed", 0);
        int64_t min_eval = m_cuba_configuration.get<int64_t>("min_eval", 0);
        int64_t max_eval = m_cuba_configuration.get<int64_t>("max_eval", 500000);
        std::string grid_file = m_cuba_configuration.get<std::string>("grid_file", "");

        // Common arguments entering the flags bitset
        uint8_t verbosity = m_cuba_configuration.get<int64_t>("verbosity", 0);
        bool subregion = m_cuba_configuration.get<bool>("subregion", false);
        bool retainStateFile = m_cuba_configuration.get<bool>("retainStateFile", false);
        uint64_t level = m_cuba_configuration.get<int64_t>("level", 0);
        // Only used by vegas!
        bool takeOnlyGridFromFile = m_cuba_configuration.get<bool>("takeOnlyGridFromFile", true);
        // Only used by vegas and suave!
        bool smoothing = m_cuba_configuration.get<bool>("smoothing", true);

        unsigned int flags = cuba::createFlagsBitset(verbosity, subregion, retainStateFile, level, smoothing, takeOnlyGridFromFile);

        int64_t ncores = m_cuba_configuration.get<int64_t>("ncores", 0);
        int64_t pcores = m_cuba_configuration.get<int64_t>("pcores", 1000000);
        cubacores(ncores, pcores);

        // Output from cuba
        long long int neval = 0;
        int nfail = 0;
        std::unique_ptr<double[]> prob(new double[m_n_components]);

        for (size_t i = 0; i < m_n_components; i++) {
            prob[i] = 0;
        }

        if (algorithm == "vegas") {
            int64_t n_start = m_cuba_configuration.get<int64_t>("n_start", 25000);
            int64_t n_increase = m_cuba_configuration.get<int64_t>("n_increase", 0);
            int64_t batch_size = m_cuba_configuration.get<int64_t>("batch_size", n_start);
            int64_t grid_number = m_cuba_configuration.get<int64_t>("grid_number", 0);

            llVegas(
                    m_n_dimensions,         // (int) dimensions of the integrated volume
                    m_n_components,         // (int) dimensions of the integrand
                    (integrand_t) CUBAIntegrandWeighted,  // (integrand_t) integrand (cast to integrand_t)
                    (void *) this,           // (void*) pointer to additional arguments passed to integrand
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
                    nullptr,                   // (int*) "spinning cores": -1 || NULL <=> integrator takes care of starting & stopping child processes (other value => keep or retrieve child processes, probably not useful here)
                    &neval,                 // (int*) actual number of evaluations done
                    &nfail,                 // 0=desired accuracy was reached; -1=dimensions out of range; >0=accuracy was not reached
                    mcResult.get(),         // (double*) integration result ([ncomp])
                    error.get(),            // (double*) integration error ([ncomp])
                    prob.get()              // (double*) Chi-square p-value that error is not reliable (ie should be <0.95) ([ncomp])
            );
        } else if (algorithm == "suave") {
            int64_t n_new = m_cuba_configuration.get<int64_t>("n_new", 1000);
            int64_t n_min = m_cuba_configuration.get<int64_t>("n_min", 2);
            double flatness = m_cuba_configuration.get<double>("flatness", 0.25);

            int nregions = 0;

            llSuave(
                    m_n_dimensions,
                    m_n_components,
                    (integrand_t) CUBAIntegrandWeighted,
                    (void *) this,
                    1,
                    relative_accuracy,
                    absolute_accuracy,
                    flags,
                    seed,
                    min_eval,
                    max_eval,
                    n_new,
                    n_min,
                    flatness,
                    grid_file.c_str(),
                    nullptr,
                    &nregions,
                    &neval,
                    &nfail,
                    mcResult.get(),
                    error.get(),
                    prob.get()
            );
        } else if (algorithm == "divonne") {
            int64_t key1 = m_cuba_configuration.get<int64_t>("key1", 47);
            int64_t key2 = m_cuba_configuration.get<int64_t>("key2", 1);
            int64_t key3 = m_cuba_configuration.get<int64_t>("key3", 1);
            int64_t maxpass = m_cuba_configuration.get<int64_t>("maxpass", 5);
            double border = m_cuba_configuration.get<double>("border", 0);
            double maxchisq = m_cuba_configuration.get<double>("maxchisq", 10.0);
            double mindeviation = m_cuba_configuration.get<double>("mindeviation", 0.25);

            int nregions = 0;

            llDivonne(
                    m_n_dimensions,
                    m_n_components,
                    (integrand_t) CUBAIntegrand,
                    (void *) this,
                    1,
                    relative_accuracy,
                    absolute_accuracy,
                    flags,
                    seed,
                    min_eval,
                    max_eval,
                    key1, key2, key3,
                    maxpass,
                    border,
                    maxchisq,
                    mindeviation,
                    0,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    grid_file.c_str(),
                    nullptr,
                    &nregions,
                    &neval,
                    &nfail,
                    mcResult.get(),
                    error.get(),
                    prob.get()
            );
        } else if (algorithm == "cuhre") {
            int64_t key = m_cuba_configuration.get<int64_t>("key", 0);

            int nregions = 0;

            llCuhre(
                    m_n_dimensions,
                    m_n_components,
                    (integrand_t) CUBAIntegrand,
                    (void *) this,
                    1,
                    relative_accuracy,
                    absolute_accuracy,
                    flags,
                    min_eval,
                    max_eval,
                    key,
                    grid_file.c_str(),
                    nullptr,
                    &nregions,
                    &neval,
                    &nfail,
                    mcResult.get(),
                    error.get(),
                    prob.get()
            );
        } else {
            throw cuba_configuration_error("Integration algorithm " + algorithm + " is not supported");
        }

        if (nfail == 0) {
            integration_status = IntegrationStatus::SUCCESS;
        } else if (nfail == -1) {
            integration_status = IntegrationStatus::DIM_OUT_OF_RANGE;
        } else if (nfail > 0) {
            integration_status = IntegrationStatus::ACCURACY_NOT_REACHED;
        } else if (nfail == -99) {
            integration_status = IntegrationStatus::ABORTED;
        }
    } else {

        LOG(debug) << "No integration dimension requested, bypassing integration.";

        // Directly call integrand
        int status = integrand(nullptr, mcResult.get(), nullptr);

        if (status == CUBA_OK) {
            integration_status = IntegrationStatus::SUCCESS;
        } else {
            integration_status = IntegrationStatus::ABORTED;
        }
    }

#ifdef DEBUG_TIMING
    LOG(info) << "Time spent evaluating modules (more details for loopers below):";
    for (auto it: m_module_timing) {
        LOG(info) << "    " << it.first->name() << ": " << duration_cast<duration<double>>(it.second).count() << "s";
    }
#endif

    for (const auto& module: m_modules) {
        module->endIntegration();
    }

    std::vector<std::pair<double, double>> result;
    for (size_t i = 0; i < m_n_components; i++) {
        result.push_back( std::make_pair(mcResult[i], error[i]) );
    }

    return result;
}

int MoMEMta::integrand(const double* psPoints, double* results, const double* weights=nullptr) {

    // Store phase-space points into the pool
    std::memcpy(m_ps_points->data(), psPoints, sizeof(double) * m_n_dimensions);

    if (weights != nullptr) {
        // Store phase-space weight into the pool
        *m_ps_weight = *weights;
    }

    for (auto& module: m_modules)
        module->beginPoint();

    for (auto& module: m_modules) {
#ifdef DEBUG_TIMING
        auto start = high_resolution_clock::now();
#endif
        auto status = module->work();
#ifdef DEBUG_TIMING
        m_module_timing[module.get()] += high_resolution_clock::now() - start;
#endif

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

    for (auto& module: m_modules)
        module->endPoint();

    for (size_t i = 0; i < m_n_components; i++)
        results[i] = *(m_integrands[i]);

    return CUBA_OK;
}

int MoMEMta::CUBAIntegrand(const int *nDim, const double* psPoint, const int *nComp, double *value, void *inputs, const int *nVec, const int *core) {
    UNUSED(nDim);
    UNUSED(nComp);
    UNUSED(nVec);
    UNUSED(core);

    return static_cast<MoMEMta*>(inputs)->integrand(psPoint, value);
}

int MoMEMta::CUBAIntegrandWeighted(const int *nDim, const double* psPoint, const int *nComp, double *value, void *inputs, const int *nVec, const int *core, const double *weight) {
    UNUSED(nDim);
    UNUSED(nComp);
    UNUSED(nVec);
    UNUSED(core);

    return static_cast<MoMEMta*>(inputs)->integrand(psPoint, value, weight);
}

void MoMEMta::cuba_logging(const char* s) {
    std::stringstream ss(s);
    std::string line;

    while(std::getline(ss, line, '\n')) {
        if (line.length() > 0)
            LOG(debug) << line;
    }
}

MoMEMta::IntegrationStatus MoMEMta::getIntegrationStatus() const {
    return integration_status;
}

void MoMEMta::checkIfPhysical(const LorentzVector& p4) {
    // Use M2() to prevent computation of the square root
    if ((p4.M2() < 0) || (p4.E() < 0)) {
        auto exception = unphysical_lorentzvector_error(p4);
        LOG(fatal) << exception.what();
        throw exception;
    }
}

MoMEMta::unphysical_lorentzvector_error::unphysical_lorentzvector_error(const LorentzVector& p4) {
    std::stringstream msg;

    msg << "Unphysical lorentz vector: " << p4;
    msg << ". Please ensure that the energy and the mass are positive or null.";

    _what = msg.str();
}

const char* MoMEMta::unphysical_lorentzvector_error::what() const noexcept {
    return _what.c_str();
}