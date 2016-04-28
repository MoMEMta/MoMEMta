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

/** \brief Transfer function on energy described by a Gaussian distribution
 *
 * This module takes as inputs a LorentzVector and a phase-space point, generates
 * a new LorentzVector with a different energy (keeping direction and invariant mass),
 * and evaluates the transfer function on the "reconstructed" and "generated" energies.
 *
 * The transfer function (TF) is a Gaussian distribution that describes the difference between 
 * the reconstructed and the generated energy (\f$E_{rec}-E_{gen}\f$). The width of the distribution depends 
 * on the "reconstructed" energy and it is set as parameter. 
 *
 *  - Integration dimension: 1
 *
 *  - Parameters:
 *    - `sigma` (double): Fraction of the "reconstructed" energy corresponding to the sigma of the Gaussian distribution. 
 *    - `sigma_range` (double): Range of integration expressed in times of sigma. 
 *
 *  - Inputs:
 *    - `ps_point` (double): Phase-space point generated by CUBA.
 *    - `reco_particle` (LorentzVector): Input LorentzVector (experimentally reconstructed particle).
 *
 *  - Outputs:
 *    - `output` (LorentzVector): Output "generated" LorentzVector, only differing from 'reco_particle' by its energy.
 *    - `TF_times_jacobian` (double): Product of the TF evaluated on the 'reco' and 'gen' energies, 
 *                                    times the jacobian of the transformation needed stretch the integration range 
 *                                    from \f$[0,1]\f$ to the width of the TF, 
 *                                    times the jacobian \f$dE/d|P|\f$ due to the fact that the integration is done 
 *                                    w.r.t \f$|P|\f$, while the TF is parametrised in terms of energy.
 * 
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
