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
#include <momemta/Solution.h>
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
 * each of those solutions, along with the rest of the particles in the event. To define the integrand,
 * these evaluations are then summed together.
 *
 * This module loops over the solutions given as input and produces a vector of quantities which will be
 * summed by MoMEMta to define the final integrand.
 *
 * ### In more details
 *
 * In the following, the set of solutions will be indexed by \f$j\f$. Particles produced by the Block will be
 * called 'invisible', while other, uniquely defined particles in the event will be called 'visible'. Since initial
 * state momenta are computed from the whole event, they have the same multiplicity as the 'invisibles' and will therefore
 * also be indexed by \f$j\f$.
 *
 * If no invisibles are present and the matrix element only has to be evaluated on the uniquely defined visible particles,
 * this module can still be used to define the integrand: no loop is done in this case, and the index \f$j\f$ can be omitted
 * in the following.
 *
 * As stated above, this modules's ouput is a vector \f$\vec{I}\f$ whose entries are:
 * \f[
 *      I_j = \frac{1}{2 x_1^j x_2^j s} \times \left( \sum_{i_1, i_2} \, f(i_1, x_1^j, Q_f^2) \, f(i_2, x_2^j, Q_f^2) \, \left| \mathcal{M}(i_1, i_2, j) \right|^2 \right) \times \left( \mathcal{J}^{\text{inv.}}_j \, \prod_{i} \mathcal{J}_i \right) \times \left( \prod_{i \in \text{vis.}} \frac{\left|p_i\right|^2 \sin(\theta_i)}{2 E_i(2\pi)^3} \right)
 * \f]
 * where:
 *    - \f$s\f$ is the hadronic centre-of-mass energy.
 *    - \f$\mathcal{J}_i\f$ are the jacobians not associated to the Block.
 *    - \f$x_1^j, x_2^j\f$ are the initial particles' Björken fractions (computed from the entry \f$j\f$ of the initial states given as input).
 *    - \f$f(i, x^j, Q_f^2)\f$ is the PDF of parton flavour \f$i\f$ evaluated on the initial particles' Björken-\f$x\f$ and using factorisation scale \f$Q_f\f$.
 *    - \f$\left| \mathcal{M}(i_1, i_2, j) \right|^2\f$ is the matrix element squared evaluated on all the particles' momenta in the event, for solution \f$j\f$. Along with the PDFs, a sum is done over all the initial parton flavours \f$i_1, i_2\f$ defined by the matrix element.
 *    - \f$\mathcal{J}^{\text{inv.}}_j\f$ is the Block's jacobian for solution \f$j\f$.
 *
 * ### Expected parameter sets
 *
 * Some inputs expected by this module are not simple parameters, but sets of parameters and input tags. These are used for:
 *
 *    - matrix element:
 *      - `card` (string): Path to the the matrix element's `param_card.dat` file.
 *
 *    - 'invisible' particles:
 *      - `input` (vector(vector(LorentzVector)): Set of solutions for the invisibles, given by the Block.
 *      - `jacobians` (vector(double)): Set of jacobians given by the Block, one per invisibles' solution.
 *      - `ids`: Parameter set used to link the invisibles to the matrix element (see below).
 *
 *    - 'visible' particles:
 *      - `inputs` (vector(LorentzVector)): Set of visible particles.
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
 * means that the visible particle vector corresponds to (electron, positron), while the matrix element expects to be given first the positron, then the electron.
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
 * 
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `initialState` | vector(vector(LorentzVector)) | Sets of initial parton 4-momenta (one pair per invisibles' solution), typically coming from a BuildInitialState module. |
 *   | `invisibles` | ParameterSet | Set of parameters defining the 'invisible' particles (see above explanation). |
 *   | `particles` | ParameterSet | Set of parameters defining the 'visible' particles (see above explanation). | 
 *   | `jacobians` | vector(double) | All jacobians defined in the integration (transfer functions, generators, ...). Note: jacobians associated to Blocks are to be given in the `invisibles` parameter set. |
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

            const auto& invisibles_set = parameters.get<ParameterSet>("invisibles");

            InputTag solution_tag = invisibles_set.get<InputTag>("input");
            m_solution = get<Solution>(solution_tag);

            const auto& invisibles_ids_set = invisibles_set.get<std::vector<ParameterSet>>("ids");
            for (const auto& s: invisibles_ids_set) {
                ParticleId id;
                id.pdg_id = s.get<int64_t>("pdg_id");
                id.me_index = s.get<int64_t>("me_index");
                m_invisibles_ids.push_back(id);
            }

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

            const auto& jacobians_tags = parameters.get<std::vector<InputTag>>("jacobians");
            for (const auto& tag: jacobians_tags) {
                m_jacobians.push_back(get<double>(tag));
            }

            std::string matrix_element = parameters.get<std::string>("matrix_element");
            const ParameterSet& matrix_element_configuration = parameters.get<ParameterSet>("matrix_element_parameters");
            m_ME = MatrixElementFactory::get().create(matrix_element, matrix_element_configuration);

            // PDF, if asked
            if (use_pdf) {
                // Silence LHAPDF
                LHAPDF::setVerbosity(0);

                std::string pdf = parameters.get<std::string>("pdf");
                m_pdf.reset(LHAPDF::mkPDF(pdf, 0));

                double pdf_scale = parameters.get<double>("pdf_scale");
                pdf_scale_squared = SQ(pdf_scale);
            }

        };

        virtual Status work() override {
            static std::vector<LorentzVector> empty_vector;

            *m_integrand = 0;

            std::vector<LorentzVector> particles(m_particles.size());
            for (size_t index = 0; index < m_particles.size(); index++) {
                particles[index] = *m_particles[index];
            }

            internal_work(*m_partons, m_solution->values, m_solution->jacobian, particles);

            return Status::OK;
        }

        virtual void internal_work(const std::vector<LorentzVector>& partons, const std::vector<LorentzVector>& invisibles, double invisibles_jacobian, const std::vector<LorentzVector> particles) {
            std::vector<std::pair<int, std::vector<double>>> finalStates;
            std::vector<int64_t> indexing;

            for (size_t i = 0; i < m_invisibles_ids.size(); i++) {
                finalStates.push_back(std::make_pair(m_invisibles_ids[i].pdg_id, toVector(invisibles[i])));
                indexing.push_back(m_invisibles_ids[i].me_index - 1);
            }

            for (size_t i = 0; i < m_particles_ids.size(); i++) {
                finalStates.push_back(std::make_pair(m_particles_ids[i].pdg_id, toVector(particles[i])));
                indexing.push_back(m_particles_ids[i].me_index - 1);
            }

            // Sort the array taking into account the indexing in the configuration
            std::vector<int64_t> suite(indexing.size());
            for (size_t i = 0; i < indexing.size(); i++)
                suite[i] = i;

            auto permutations = get_permutations(suite, indexing);
            apply_permutations(finalStates, permutations);

            std::pair<std::vector<double>, std::vector<double>> initialState { toVector(partons[0]), toVector(partons[1]) };

            auto result = m_ME->compute(initialState, finalStates);

            double x1 = std::abs(partons[0].Pz() / (sqrt_s / 2.));
            double x2 = std::abs(partons[1].Pz() / (sqrt_s / 2.));

            // Compute flux factor 1/(2*x1*x2*s)
            double phaseSpaceIn = 1. / (2. * x1 * x2 * SQ(sqrt_s));

            // Compute phase space density for observed particles (not concerned by the change of variable)
            // dPhi = |P|^2 sin(theta)/(2*E*(2pi)^3)
            double phaseSpaceOut = 1.;
            for (const auto& p: particles) {
                phaseSpaceOut *= SQ(p.P()) * sin(p.Theta()) / (2.0 * p.E() * CB(2. * M_PI));
            }

            double integrand = phaseSpaceIn * phaseSpaceOut * invisibles_jacobian;
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
        }

    private:
        double sqrt_s;
        bool use_pdf;
        double pdf_scale_squared = 0;
        std::shared_ptr<momemta::MatrixElement> m_ME;
        std::shared_ptr<LHAPDF::PDF> m_pdf;

        // Inputs
        Value<std::vector<LorentzVector>> m_partons;

        Value<Solution> m_solution;
        std::vector<ParticleId> m_invisibles_ids;

        std::vector<Value<LorentzVector>> m_particles;
        std::vector<ParticleId> m_particles_ids;

        std::vector<Value<double>> m_jacobians;

        // Outputs
        std::shared_ptr<double> m_integrand = produce<double>("output");
};
REGISTER_MODULE(MatrixElement);
