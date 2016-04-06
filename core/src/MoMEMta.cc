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

#include <logging.h>


// FIXME: Move
unsigned int setFlags(char verbosity, bool subregion, bool retainStateFile, unsigned int level, bool smoothing, bool takeOnlyGridFromFile) {
    unsigned int flags = 0;

    static unsigned int opt_subregion = 0x04; // bit 2 (=4)
    static unsigned int opt_smoothing = 0x08; // bit 3 (=8)
    static unsigned int opt_retainStateFile = 0x10; // bit 4 (=16)
    static unsigned int opt_takeOnlyGridFromFile = 0x20; // bit 5 (=32)

    level <<= 8; // bits 8-31
    flags |= level | verbosity; // verbosity: bits 0-1
    if(subregion) flags |= opt_subregion;
    if(!smoothing) flags |= opt_smoothing; // careful true-false inverted
    if(retainStateFile) flags |= opt_retainStateFile;
    if(takeOnlyGridFromFile) flags |= opt_takeOnlyGridFromFile;

    return flags;
}

MoMEMta::MoMEMta(const ConfigurationReader& configuration) {
    
    // Initialize shared memory pool for modules
    m_pool.reset(new Pool());

    // Create phase-space points vector, input for many modules
    m_ps_points = m_pool->put<std::vector<double>>({"cuba", "ps_points"});

    // Create vector for input particles
    m_particles = m_pool->put<std::vector<LorentzVector>>({"input", "particles"});

    // Construct modules from configuration
    std::vector<LightModule> modules = configuration.getModules();
    for (const auto& module: modules) {
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
    m_weights = m_pool->get<std::vector<double>>({m_modules.back()->name(), "weights"});

    cubacores(0, 0);
}

MoMEMta::~MoMEMta() {
    for (const auto& module :m_modules) {
        module->finish();
    }
}

std::vector<std::pair<double, double>> MoMEMta::computeWeights(const std::vector<LorentzVector>& particules) {

    *m_particles = particules;

    int neval, nfail;
    double mcResult = 0, prob = 0, error = 0;

    char verbosity = 3; // 0-3
    bool subregion = false; // true = only last set of samples is used for final evaluation of integral
    bool smoothing = false;
    bool retainStateFile = false; // false => delete state file when integration ends
    bool takeOnlyGridFromFile = true; // false => full state taken from file (if present), true => only grid is taken (e.g. to use it for another integrand)
    unsigned int level = 0;

    unsigned int flags = setFlags(verbosity, subregion, retainStateFile, level, smoothing, takeOnlyGridFromFile);

    Vegas
        (
         m_n_dimensions,         // (int) dimensions of the integrated volume
         1,                      // (int) dimensions of the integrand
         (integrand_t) CUBAIntegrand,  // (integrand_t) integrand (cast to integrand_t)
         (void*) this,           // (void*) pointer to additional arguments passed to integrand
         1,                      // (int) maximum number of points given the integrand in each invocation (=> SIMD) ==> PS points = vector of sets of points (x[ndim][nvec]), integrand returns vector of vector values (f[ncomp][nvec])
         0.005,                  // (double) requested relative accuracy  /
         0.,                     // (double) requested absolute accuracy /-> error < max(rel*value,abs)
         flags,                  // (int) various control flags in binary format, see setFlags function
         0,                      // (int) seed (seed==0 => SOBOL; seed!=0 && control flag "level"==0 => Mersenne Twister)
         0,                      // (int) minimum number of integrand evaluations
         500000,                    // (int) maximum number of integrand evaluations (approx.!)
         25000,                     // (int) number of integrand evaluations per interations (to start)
         0,                      // (int) increase in number of integrand evaluations per interations
         12500,                   // (int) batch size for sampling
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
    *value = static_cast<MoMEMta*>(inputs)->integrand(psPoint, weight);

    return 0;
}
