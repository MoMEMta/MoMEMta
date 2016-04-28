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


#include <momemta/ParameterSet.h>
#include <momemta/MatrixElement.h>
#include <momemta/MatrixElementFactory.h>
#include <momemta/Module.h>
#include <momemta/Types.h>
#include <momemta/Utils.h>

#include <logging.h>

#include <LHAPDF/LHAPDF.h>

class MatrixElement: public Module {
    struct ParticleId {
        int64_t pdg_id;
        int64_t me_index;
    };

    public:

        MatrixElement(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {

            sqrt_s = parameters.globalParameters().get<double>("energy");
            M_T = parameters.globalParameters().get<double>("top_mass");
            
            use_pdf = parameters.get<bool>("use_pdf", true);

            m_partons = get<std::vector<std::vector<LorentzVector>>>(parameters.get<InputTag>("initialState"));

            const auto& invisibles_set = parameters.get<ParameterSet>("invisibles");

            InputTag invisibles_tag = invisibles_set.get<InputTag>("input");
            LOG(debug) << "[MatrixElement] invisibles input tag: " << invisibles_tag.toString();
            m_invisibles = get<std::vector<std::vector<LorentzVector>>>(invisibles_tag);

            const auto& invisibles_jacobians_tag = invisibles_set.get<InputTag>("jacobians");
            LOG(debug) << "[MatrixElement] invisibles jacobians tag: " << invisibles_jacobians_tag.toString();
            m_invisibles_jacobians = get<std::vector<double>>(invisibles_jacobians_tag);

            const auto& invisibles_ids_set = invisibles_set.get<std::vector<ParameterSet>>("ids");
            LOG(debug) << "[MatrixElement] # invisibles ids: " << invisibles_ids_set.size();
            for (const auto& s: invisibles_ids_set) {
                ParticleId id;
                id.pdg_id = s.get<int64_t>("pdg_id");
                id.me_index = s.get<int64_t>("me_index");
                m_invisibles_ids.push_back(id);
            }

            const auto& particles_set = parameters.get<ParameterSet>("particles");

            m_particles_tags = particles_set.get<std::vector<InputTag>>("inputs");
            for (auto& tag: m_particles_tags)
                tag.resolve(pool);
            LOG(debug) << "[MatrixElement] # particles input tags: " << m_particles_tags.size();

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
            if(use_pdf){
                // Silence LHAPDF
                LHAPDF::setVerbosity(0);
                std::string pdf = parameters.get<std::string>("pdf");
                m_pdf.reset(LHAPDF::mkPDF(pdf, 0));
            }

        };

        virtual void work() override {
            static std::vector<LorentzVector> empty_vector;

            m_integrands->clear();

            uint64_t n_sols = m_invisibles->size();

            if (!m_invisibles_ids.empty() && !n_sols)
                return;

            std::vector<LorentzVector> particles(m_particles_tags.size());
            for (size_t index = 0; index < m_particles_tags.size(); index++) {
                particles[index] = m_particles_tags[index].get<LorentzVector>();
            }

            // Loop over all solutions if there's, otherwise use only particles
            if (!m_invisibles.get() || m_invisibles->empty()) {
                // No invisibles particles. Use only particles for the matrix element
                // computation
                internal_work((*m_partons)[0], empty_vector, 1, particles);
            } else {
                // Loop over all invisibles solutions
                for (size_t i = 0; i < m_invisibles->size(); i++) {
                    internal_work((*m_partons)[i], (*m_invisibles)[i], (*m_invisibles_jacobians)[i], particles);
                }
            }
            
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

            std::vector<std::vector<double>> initialState(partons.size());
            size_t index = 0;
            for (const auto& parton: partons)
                initialState[index++] = toVector(parton);

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
                double pdf1 = use_pdf ? m_pdf->xfxQ2(me.first.first, x1, SQ(M_T)) / x1 : 1;
                double pdf2 = use_pdf ? m_pdf->xfxQ2(me.first.second, x2, SQ(M_T)) / x2 : 1;

                final_integrand += me.second * pdf1 * pdf2;
            }

            final_integrand *= integrand;
            m_integrands->push_back(final_integrand);
        }

    private:
        double sqrt_s;
        double M_T;
        bool use_pdf;

        std::shared_ptr<const std::vector<std::vector<LorentzVector>>> m_partons;

        std::shared_ptr<const std::vector<std::vector<LorentzVector>>> m_invisibles;
        std::shared_ptr<const std::vector<double>> m_invisibles_jacobians;
        std::vector<ParticleId> m_invisibles_ids;

        std::vector<InputTag> m_particles_tags;
        std::vector<ParticleId> m_particles_ids;

        std::vector<std::shared_ptr<const double>> m_jacobians;

        std::shared_ptr<momemta::MatrixElement> m_ME;

        std::shared_ptr<LHAPDF::PDF> m_pdf;

        std::shared_ptr<std::vector<double>> m_integrands = produce<std::vector<double>>("integrands");
};
REGISTER_MODULE(MatrixElement);
