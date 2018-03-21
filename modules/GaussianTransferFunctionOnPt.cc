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


#include <momemta/Logging.h>
#include <momemta/Module.h>
#include <momemta/ParameterSet.h>
#include <momemta/Types.h>
#include <momemta/Math.h>

#include <Math/DistFunc.h>

/** \brief Helper class for Gaussian transfer function modules
 *
 * Base class helping to define TF modules having different behaviours (allowing either to integrate over a TF, or simply evaluate it).
 *
 * \sa GaussianTransferFunctionOnPt
 * \sa GaussianTransferFunctionOnPtEvaluator
 */
class GaussianTransferFunctionOnPtBase: public Module {
    public:

        GaussianTransferFunctionOnPtBase(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            m_reco_input = get<LorentzVector>(parameters.get<InputTag>("reco_particle"));

            m_sigma = parameters.get<double>("sigma", 0.10);
            m_sigma_range = parameters.get<double>("sigma_range", 5);
            m_min_Pt = parameters.get<double>("min_Pt", 0.);
        }

    protected:
        double m_min_Pt;
        double m_sigma;
        double m_sigma_range;

        // Input
        Value<LorentzVector> m_reco_input;
};

/** \brief Integrate over a transfer function on Pt described by a Gaussian distribution
 *
 * This module takes as inputs a LorentzVector and a phase-space point, generates
 * a new LorentzVector with a different Pt (keeping direction and invariant mass),
 * and evaluates the transfer function on the "reconstructed" and "generated" Pt.
 *
 * The transfer function (TF) is a Gaussian distribution that describes the difference between
 * the reconstructed and the generated Pt (\f$P_{T_{rec}}-P_{T_{gen}}\f$). The width of the distribution, parametrised as a fraction of \f$P_{T_{gen}}\f$, is set as parameter.
 *
 * The range of the integration is determined using the width of the Gaussian at \f$P_{T_{rec}}\f$, integrating over a user-defined 'number of sigmas' `n`: \f$P_{T_{gen}} \in \pm n \cdot \sigma \cdot P_{T_{rec}}\f$.
 *
 * ### Integration dimension
 *
 * This module requires **1** phase-space point.
 *
 * ### Parameters
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `sigma` | double | Fraction of the Pt yielding the width of the Gaussian distribution (with `sigma` at `0.1`, \f$\sigma_{gauss} = 0.1 \cdot P_{T_{gen}}\f$). |
 *   | `sigma_range` | double | Range of integration expressed in number of sigma. |
 *   | `min_Pt` | double | Optional: cut on Pt to avoid divergences |
 *
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `ps_point` | double | Phase-space point generated by CUBA. |
 *   | `reco_particle` | LorentzVector | Input LorentzVector (experimentally reconstructed particle). |
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `output` | LorentzVector | Output *generated* LorentzVector, only differing from *reco_particle* by its Pt. |
 *   | `TF_times_jacobian` | double | Product of the TF evaluated on the *reco* and *gen* energies, times the jacobian of the transformation needed stretch the integration range from \f$[0,1]\f$ to the width of the TF, times the jacobian \f$d|P|/dP_T\f$ due to the fact that the integration is done w.r.t \f$|P|\f$, while the TF is parametrised in terms of Pt. |
 *
 * \ingroup modules
 * \sa GaussianTransferFunctionOnPtEvaluator
 */
class GaussianTransferFunctionOnPt: public GaussianTransferFunctionOnPtBase {
    public:
        GaussianTransferFunctionOnPt(PoolPtr pool, const ParameterSet& parameters): GaussianTransferFunctionOnPtBase(pool, parameters) {
            m_ps_point = get<double>(parameters.get<InputTag>("ps_point"));
        }

        virtual Status work() override {
            // Estimate the width over which to integrate using the width of the TF at Pt_rec ...
            const double sigma_Pt_rec = m_reco_input->Pt() * m_sigma;

            const double cosh_eta = std::cosh(m_reco_input->Eta());
            double range_min = std::max(m_min_Pt, m_reco_input->Pt() - (m_sigma_range * sigma_Pt_rec));
            double range_max = m_reco_input->Pt() + (m_sigma_range * sigma_Pt_rec);
            double range = (range_max - range_min);

            double gen_Pt = range_min + range * (*m_ps_point);

            // To change the particle's Pt without changing its direction and mass:
            const double gen_E = std::sqrt(SQ(m_reco_input->M()) + SQ(cosh_eta * gen_Pt));

            output->SetCoordinates(
                    gen_Pt * std::cos(m_reco_input->Phi()),
                    gen_Pt * std::sin(m_reco_input->Phi()),
                    gen_Pt * std::sinh(m_reco_input->Eta()),
                    gen_E);

            // ... but compute the width of the TF at Pt_gen!
            const double sigma_Pt_gen = gen_Pt * m_sigma;

            // Compute TF*jacobian, where the jacobian includes the transformation of [0,1]->[range_min,range_max] and d|P|/dPt = cosh(eta)
            *TF_times_jacobian = ROOT::Math::normal_pdf(gen_Pt, sigma_Pt_gen, m_reco_input->Pt()) * range * cosh_eta;

            return Status::OK;
        }

    private:
        // Input
        Value<double> m_ps_point;

        // Outputs
        std::shared_ptr<LorentzVector> output = produce<LorentzVector>("output");
        std::shared_ptr<double> TF_times_jacobian = produce<double>("TF_times_jacobian");

};

/** \brief Evaluate a transfer function on Pt described by a Gaussian distribution
 *
 * This module takes as inputs two LorentzVectors: a 'gen-level' particle (which may be computed using for instance a Block or a 'real' transfer function) and a 'reco-level' particle (experimentally reconstructed).
 * Assuming the LorentzVectors differ only by their Pt, this module returns the value of a transfer function (TF) evaluated on their respective Pt.
 *
 * The TF is a Gaussian distribution that describes the difference between the reconstructed and the generated Pt (\f$P_{T_{rec}}-P_{T_{gen}}\f$). The width of the distribution, parametrised as a fraction of \f$P_{T_{gen}}\f$, is set as parameter.
 *
 * ### Integration dimension
 *
 * This module requires **0** phase-space points.
 *
 * ### Parameters
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `sigma` | double | Fraction of the Pt yielding the width of the Gaussian distribution (with `sigma` at `0.1`, \f$\sigma_{gauss} = 0.1 \cdot P_{T_{gen}}\f$). |
 * 
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `reco_particle` | LorentzVector | Experimentally reconstructed particle. |
 *   | `gen_particle` | LorentzVector | Gen-level particle. |
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `TF` | double | Value of the TF evaluated on the *reco* and *gen* Pt. |
 *
 * \ingroup modules
 * \sa GaussianTransferFunctionOnPt
 */
class GaussianTransferFunctionOnPtEvaluator: public GaussianTransferFunctionOnPtBase {
    public:
        GaussianTransferFunctionOnPtEvaluator(PoolPtr pool, const ParameterSet& parameters): GaussianTransferFunctionOnPtBase(pool, parameters) {
            m_gen_input = get<LorentzVector>(parameters.get<InputTag>("gen_particle"));
        }

        virtual Status work() override {
            // Compute TF value
            *TF_value = ROOT::Math::normal_pdf(m_gen_input->Pt(), m_gen_input->Pt() * m_sigma, m_reco_input->Pt());

            return Status::OK;
        }

    private:
        // Input
        Value<LorentzVector> m_gen_input;

        // Outputs
        std::shared_ptr<double> TF_value = produce<double>("TF");

};

REGISTER_MODULE(GaussianTransferFunctionOnPt)
        .Input("ps_point")
        .Input("reco_particle")
        .Output("output")
        .Output("TF_times_jacobian")
        .Attr("sigma:double=0.10")
        .Attr("sigma_range:double=5")
        .Attr("min_Pt:double=0");

REGISTER_MODULE(GaussianTransferFunctionOnPtEvaluator)
        .Input("gen_particle")
        .Input("reco_particle")
        .Output("TF")
        .Attr("sigma:double=0.10")
        .Attr("sigma_range:double=5")
        .Attr("min_Pt:double=0");