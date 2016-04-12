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


#include <cstring>

#include <cuba.h>

#include <momemta/ConfigurationReader.h>
#include <momemta/MoMEMta.h>
#include <momemta/Utils.h>

#include <logging.h>

MoMEMta::MoMEMta(const ConfigurationReader& configuration) {
    
    // Initialize shared memory pool for modules
    m_pool.reset(new Pool());

    // Create phase-space points vector, input for many modules
    m_pool->current_module("cuba");
    m_ps_points = m_pool->put<std::vector<double>>({"cuba", "ps_points"});

    // Create vector for input particles
    m_pool->current_module("input");
    m_particles = m_pool->put<std::vector<LorentzVector>>({"input", "particles"});

    // Construct modules from configuration
    std::vector<LightModule> modules = configuration.getModules();
    for (const auto& module: modules) {
        m_pool->current_module(module.name);
        m_modules.push_back(ModuleFactory::get().create(module.type, m_pool, *module.parameters));
        m_modules.back()->configure();
    }

    m_n_dimensions = 0;
    for (const auto& module: m_modules) {
        m_n_dimensions += module->dimensions();
    }

    LOG(info) << "Number of dimensions for integration: " << m_n_dimensions;

    // Resize pool ps-points vector
    m_ps_points->resize(m_n_dimensions);

    // The last module of the chain *must* output a vector of weights, used for the integration
    m_pool->current_module("momemta");
    m_weights = m_pool->get<std::vector<double>>({m_modules.back()->name(), "weights"});

    m_pool->freeze();

    m_vegas_configuration = configuration.getVegasConfiguration();

    cubacores(0, 0);
}

MoMEMta::~MoMEMta() {
    for (const auto& module: m_modules) {
        module->finish();
    }
}

std::vector<std::pair<double, double>> MoMEMta::computeWeights(const std::vector<LorentzVector>& particules) {

    *m_particles = particules;

    int neval, nfail;
    double mcResult = 0, prob = 0, error = 0;

    // Read vegas configuration
    uint8_t verbosity = m_vegas_configuration->get<int64_t>("verbosity", 0);
    bool subregion = m_vegas_configuration->get<bool>("subregion", false);
    bool smoothing = m_vegas_configuration->get<bool>("smoothing", false);
    bool retainStateFile = m_vegas_configuration->get<bool>("retainStateFile", false);
    bool takeOnlyGridFromFile = m_vegas_configuration->get<bool>("takeOnlyGridFromFile", true);
    uint64_t level = m_vegas_configuration->get<int64_t>("level", 0);

    double relative_accuracy = m_vegas_configuration->get<double>("relative_accuracy", 0.005);
    double absolute_accuracy = m_vegas_configuration->get<double>("absolute_accuracy", 0.);
    int64_t seed = m_vegas_configuration->get<int64_t>("seed", 0);
    int64_t min_eval = m_vegas_configuration->get<int64_t>("min_eval", 0);
    int64_t max_eval = m_vegas_configuration->get<int64_t>("max_eval", 500000);
    int64_t n_start = m_vegas_configuration->get<int64_t>("n_start", 25000);
    int64_t n_increase = m_vegas_configuration->get<int64_t>("n_increase", 0);
    int64_t batch_size = m_vegas_configuration->get<int64_t>("batch_size", 12500);

    unsigned int flags = vegas::createFlagsBitset(verbosity, subregion, retainStateFile, level, smoothing, takeOnlyGridFromFile);

    Vegas(
         m_n_dimensions,         // (int) dimensions of the integrated volume
         1,                      // (int) dimensions of the integrand
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
         0,                      // (int) grid number, 1-10 => up to 10 grids can be stored, and re-used for other integrands (provided they are not too different)
         "",                     // (char*) name of state file => state can be stored and retrieved for further refinement
         NULL,                   // (int*) "spinning cores": -1 || NULL <=> integrator takes care of starting & stopping child processes (other value => keep or retrieve child processes, probably not useful here)
         &neval,                 // (int*) actual number of evaluations done
         &nfail,                 // 0=desired accuracy was reached; -1=dimensions out of range; >0=accuracy was not reached
         &mcResult,              // (double*) integration result ([ncomp])
         &error,                 // (double*) integration error ([ncomp])
         &prob                   // (double*) Chi-square p-value that error is not reliable (ie should be <0.95) ([ncomp])
    );

    return std::vector<std::pair<double, double>>({{mcResult, error}});
}

double MoMEMta::integrand(const double* psPoints, const double* weights) {

    UNUSED(weights);

    // Store phase-space points into the pool
    std::memcpy(m_ps_points->data(), psPoints, sizeof(double) * m_n_dimensions);

    for (auto& module: m_modules) {
        module->work();
    }

    double sum = 0;
    for (const auto& p: *m_weights) {
        sum += p;
    }

    return sum;
}

int MoMEMta::CUBAIntegrand(const int *nDim, const double* psPoint, const int *nComp, double *value, void *inputs, const int *nVec, const int *core, const double *weight) {
    UNUSED(nDim);
    UNUSED(nComp);
    UNUSED(nVec);
    UNUSED(core);

    *value = static_cast<MoMEMta*>(inputs)->integrand(psPoint, weight);

    return 0;
}
