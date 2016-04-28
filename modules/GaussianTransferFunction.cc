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


#include <Math/DistFunc.h>

#include <momemta/ParameterSet.h>
#include <momemta/Module.h>
#include <momemta/Types.h>
#include <momemta/Utils.h>

#include <logging.h>

/**
 * \ingroup modules
 */
class GaussianTransferFunction: public Module {
    public:

        GaussianTransferFunction(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            m_ps_point = parameters.get<InputTag>("ps_point");
            m_input = parameters.get<InputTag>("reco_particle");

            m_ps_point.resolve(pool);
            m_input.resolve(pool);

            m_sigma = parameters.get<double>("sigma", 0.10);
            m_sigma_range = parameters.get<double>("sigma_range", 5);
        };

        virtual void work() override {

            const double& ps_point = m_ps_point.get<double>();
            const LorentzVector& reco_particle = m_input.get<LorentzVector>();

            double sigma = reco_particle.E() * m_sigma;

            double range_min = std::max(0., reco_particle.E() - (m_sigma_range * sigma));
            double range_max = reco_particle.E() + (m_sigma_range * sigma);
            double range = (range_max - range_min);

            double gen_E = range_min + (range * ps_point);
            double gen_pt = std::sqrt(SQ(gen_E) - SQ(reco_particle.M())) / std::cosh(reco_particle.Eta());

            output->SetCoordinates(
                    gen_pt * std::cos(reco_particle.Phi()),
                    gen_pt * std::sin(reco_particle.Phi()),
                    gen_pt * std::sinh(reco_particle.Eta()),
                    gen_E);

            // Compute jacobian
            *TF_times_jacobian = ROOT::Math::normal_pdf(gen_E, reco_particle.E(), sigma) * range * dE_over_dP(*output);
        }

        virtual size_t dimensions() const override {
            return 1;
        }

    private:
        InputTag m_ps_point;
        InputTag m_input;

        double m_sigma;
        double m_sigma_range;

        std::shared_ptr<LorentzVector> output = produce<LorentzVector>("output");
        std::shared_ptr<double> TF_times_jacobian = produce<double>("TF_times_jacobian");

};
REGISTER_MODULE(GaussianTransferFunction);
