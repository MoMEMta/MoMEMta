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

// Must be loaded before `momemta/Logging.h`, otherwise there's conflict between usage
// of `log()` and `namespace log`
#include <LHAPDF/LHAPDF.h>

#include <momemta/Logging.h>
#include <momemta/MatrixElement.h>
#include <momemta/MatrixElementFactory.h>
#include <momemta/ParameterSet.h>
#include <momemta/Math.h>
#include <momemta/Module.h>
#include <momemta/Types.h>
#include <momemta/Utils.h>

/** \brief Compute the integrand: matrix element, PDFs, jacobians
 *
 * ### Summary
 *
 * Evaluate the matrix element, parton density functions, jacobians, 
 * phase-space density terms, flux factor, ..., to define the final quantity to be integrated.
 *
 * The matrix element has to be evaluated on all the initial and final particles' 4-momenta.
 * In most cases, a subset of those particles are given by a Block. Blocks produce several
 * equivalent solutions for those particles, and the matrix element, ..., has to be computed on
 * each of those solutions, along with the rest of the particles in the event. For that, you need
 * to use a Looper module, with this module in the execution path.
 *
 * To define the integrand, the quantities produced by this module (one quantity per solution) has to be summed. Use the
 * DoubleSummer module for this purpose.
 *
 * ### In more details
 *
 * In the following, the set of solutions will be indexed by \f$j\f$. Particles produced by the Block will be
 * called 'invisible', while other, uniquely defined particles in the event will be called 'visible'. Since initial
 * state momenta are computed from the whole event, they have the same multiplicity as the 'invisibles' and will therefore
 * also be indexed by \f$j\f$.
 *
 * \warning Keep in mind that the loop describe below is **not** done by this module. A Looper must be used for this.
 *
 * If no invisibles are present and the matrix element only has to be evaluated on the uniquely defined visible particles,
 * this module can still be used to define the integrand: no loop is done in this case, and the index \f$j\f$ can be omitted
 * in the following.
 *
 * As stated above, for each solution \f$j\f$, this modules's ouput is a scalar \f$I\f$:
 * \f[
 *      I_j = \frac{1}{2 x_1^j x_2^j s} \times \left( \sum_{i_1, i_2} \, f(i_1, x_1^j, Q_f^2) \, f(i_2, x_2^j, Q_f^2) \, \left| \mathcal{M}(i_1, i_2, j) \right|^2 \right) \times \left( \prod_{i} \mathcal{J}_i \right)
 * \f]
 * where:
 *    - \f$s\f$ is the hadronic centre-of-mass energy.
 *    - \f$\mathcal{J}_i\f$ are the jacobians.
 *    - \f$x_1^j, x_2^j\f$ are the initial particles' Björken fractions (computed from the entry \f$j\f$ of the initial states given as input).
 *    - \f$f(i, x^j, Q_f^2)\f$ is the PDF of parton flavour \f$i\f$ evaluated on the initial particles' Björken-\f$x\f$ and using factorisation scale \f$Q_f\f$.
 *    - \f$\left| \mathcal{M}(i_1, i_2, j) \right|^2\f$ is the matrix element squared evaluated on all the particles' momenta in the event, for solution \f$j\f$. Along with the PDFs, a sum is done over all the initial parton flavours \f$i_1, i_2\f$ defined by the matrix element.
 *
 * ### Expected parameter sets
 *
 * Some inputs expected by this module are not simple parameters, but sets of parameters and input tags. These are used for:
 *
 *    - matrix element:
 *      - `card` (string): Path to the the matrix element's `param_card.dat` file.
 *
 *    - particles:
 *      - `inputs` (vector(LorentzVector)): Set of particles.
 *      - `ids`: Parameter set used to link the visibles to the matrix element (see below).
 *
 * ### Linking inputs and matrix element
 *
 * The matrix element expects the final-state particles' 4-momenta to be given in a certain order, 
 * but it is agnostic as to how the particles are ordered in MoMEMta. 
 * It is therefore necessary to specify the index of each input (visible and invisible) 
 * particle in the matrix element call. Furthermore, since the matrix element library might define several final states,
 * each input particle's PDG ID has to be set by the user, to ensure the correct final state is retrieved when evaluating the matrix element.
 *
 * To find out the ordering the matrix element expects, it is currently necessary to dig into the matrix element's code. 
 * For instance, for the fully leptonic \f$t\overline{t}\f$ example shipped with MoMEMta, the ordering and PDG IDs can be read from [here](https://github.com/MoMEMta/MoMEMta/blob/master/MatrixElements/pp_ttx_fully_leptonic/SubProcesses/P1_Sigma_sm_gg_mupvmbmumvmxbx/cpp_pp_ttx_fullylept.cc#L146).
 *
 * In the Lua configuation for this module, the ordering is defined through the `ids` parameter set mentioned above. For instance,
 * ```
 * particles = {
 *     inputs = { 'input::particles/1', 'input::particles/2' },
 *     ids = {
 *         {
 *             me_index = 2,
 *             pdg_id = 11
 *         },
 *         {
 *             me_index = 1,
 *             pdg_id = -11
 *         }
 *     }
 * }
 * ```
 * means that the particle vector corresponds to (electron, positron), while the matrix element expects to be given first the positron, then the electron.
 *
 * ### Integration dimension
 *
 * This module requires **0** phase-space point.
 *
 * ### Global Parameters
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `energy` | double | Hadronic centre-of-mass energy (GeV). |
 *
 * ### Parameters
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `use_pdf` | double, default true | Evaluate PDFs and use them in the integrand. |
 *   | `pdf` | string | Name of the LHAPDF set to be used (see [full list](https://lhapdf.hepforge.org/pdfsets.html)). |
 *   | `pdf_scale` | double | Factorisation scale used when evaluating the PDFs. |
 *   | `matrix_element` | string | Name of the matrix element to be used. |
 *   | `matrix_element_parameters` | ParameterSet | Set of parameters passed to the matrix element (see above explanation). |
 *   | `override_parameters` | ParameterSet (optional) | Overrides the value of the ME parameters (usually those specified in the param card) by the ones specified. |
 * 
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `initialState` | vector(vector(LorentzVector)) | Sets of initial parton 4-momenta (one pair per invisibles' solution), typically coming from a BuildInitialState module. |
 *   | `particles` | ParameterSet | Set of parameters defining the particles (see above explanation). |
 *   | `jacobians` | vector(double) | All jacobians defined in the integration (transfer functions, generators, blocks...). |
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `integrands` | vector(double) | Vector of integrands (one per invisibles' solution). All entries in this vector will be summed by MoMEMta to define the final integrand used by Cuba to compute the integral. | 
 *
 * \ingroup modules
 */
class MatrixElement: public Module {
    struct ParticleId {
        int64_t pdg_id;
        int64_t me_index;
    };

    public:

        MatrixElement(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {

            sqrt_s = parameters.globalParameters().get<double>("energy");

            use_pdf = parameters.get<bool>("use_pdf", true);

            m_partons = get<std::vector<LorentzVector>>(parameters.get<InputTag>("initialState"));

            const auto& particles_set = parameters.get<ParameterSet>("particles");

            auto particle_tags = particles_set.get<std::vector<InputTag>>("inputs");
            for (auto& tag: particle_tags)
                m_particles.push_back(get<LorentzVector>(tag));

            LOG(debug) << "[MatrixElement] # particles input tags: " << particle_tags.size();

            const auto& particles_ids_set = particles_set.get<std::vector<ParameterSet>>("ids");
            LOG(debug) << "[MatrixElement] # particles ids: " << particles_ids_set.size();
            for (const auto& s: particles_ids_set) {
                ParticleId id;
                id.pdg_id = s.get<int64_t>("pdg_id");
                id.me_index = s.get<int64_t>("me_index");
                m_particles_ids.push_back(id);
            }

            if (m_particles.size() != m_particles_ids.size()) {
                LOG(fatal) << "The number of particles ids is not consistent with the number of particles. Did you"
                            " forget some ids?";

                throw Module::invalid_configuration("Inconsistent number of ids and number of particles");
            }

            const auto& jacobians_tags = parameters.get<std::vector<InputTag>>("jacobians");
            for (const auto& tag: jacobians_tags) {
                m_jacobians.push_back(get<double>(tag));
            }

            std::string matrix_element = parameters.get<std::string>("matrix_element");
            const ParameterSet& matrix_element_configuration = parameters.get<ParameterSet>("matrix_element_parameters");
            m_ME = MatrixElementFactory::get().create(matrix_element, matrix_element_configuration);

            if (parameters.exists("override_parameters")) {
                const ParameterSet& matrix_element_params = parameters.get<ParameterSet>("override_parameters");
                auto p = m_ME->getParameters();

                for (const auto& name: matrix_element_params.getNames()) {
                    double value = matrix_element_params.get<double>(name);
                    p->setParameter(name, value);
                }

                p->cacheParameters();
                p->cacheCouplings();
            }

            // PDF, if asked
            if (use_pdf) {
                // Silence LHAPDF
                LHAPDF::setVerbosity(0);

                std::string pdf = parameters.get<std::string>("pdf");
                m_pdf.reset(LHAPDF::mkPDF(pdf, 0));

                double pdf_scale = parameters.get<double>("pdf_scale");
                pdf_scale_squared = SQ(pdf_scale);
            }

            // Prepare arrays for sorting particles
            for (size_t i = 0; i < m_particles_ids.size(); i++) {
                indexing.push_back(m_particles_ids[i].me_index - 1);
            }

            // Pre-allocate memory for the finalState array
            finalState.resize(m_particles_ids.size());

            // Sort the array taking into account the indexing in the configuration
            std::vector<int64_t> suite(indexing.size());
            std::iota(suite.begin(), suite.end(), 0);

            permutations = get_permutations(suite, indexing);
        };

        virtual Status work() override {
            static std::vector<LorentzVector> empty_vector;

            *m_integrand = 0;
            const std::vector<LorentzVector>& partons = *m_partons;

            LorentzVectorRefCollection particles;
            for (const auto& p: m_particles)
                particles.push_back(std::ref(*p));

            for (size_t i = 0; i < m_particles_ids.size(); i++) {
                finalState[i] = std::make_pair(m_particles_ids[i].pdg_id, toVector(particles[i].get()));
            }

            // Sort the array taking into account the indexing in the configuration
            apply_permutations(finalState, permutations);

            std::pair<std::vector<double>, std::vector<double>> initialState { toVector(partons[0]),
                                                                               toVector(partons[1]) };

            auto result = m_ME->compute(initialState, finalState);

            double x1 = std::abs(partons[0].Pz() / (sqrt_s / 2.));
            double x2 = std::abs(partons[1].Pz() / (sqrt_s / 2.));

            // Compute flux factor 1/(2*x1*x2*s)
            double phaseSpaceIn = 1. / (2. * x1 * x2 * SQ(sqrt_s));

            double integrand = phaseSpaceIn;
            for (const auto& jacobian: m_jacobians) {
                integrand *= (*jacobian);
            }

            // PDF
            double final_integrand = 0;
            for (const auto& me: result) {
                double pdf1 = use_pdf ? m_pdf->xfxQ2(me.first.first, x1, pdf_scale_squared) / x1 : 1;
                double pdf2 = use_pdf ? m_pdf->xfxQ2(me.first.second, x2, pdf_scale_squared) / x2 : 1;

                final_integrand += me.second * pdf1 * pdf2;
            }

            final_integrand *= integrand;
            *m_integrand = final_integrand;

            return Status::OK;
        }

    private:
        double sqrt_s;
        bool use_pdf;
        double pdf_scale_squared = 0;
        std::shared_ptr<momemta::MatrixElement> m_ME;
        std::shared_ptr<LHAPDF::PDF> m_pdf;

        std::vector<int64_t> indexing;
        std::vector<size_t> permutations;
        std::vector<std::pair<int, std::vector<double>>> finalState;

        // Inputs
        Value<std::vector<LorentzVector>> m_partons;

        std::vector<Value<LorentzVector>> m_particles;
        std::vector<ParticleId> m_particles_ids;

        std::vector<Value<double>> m_jacobians;

        // Outputs
        std::shared_ptr<double> m_integrand = produce<double>("output");
};
REGISTER_MODULE(MatrixElement);
