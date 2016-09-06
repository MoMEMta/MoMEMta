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
#include <momemta/Module.h>
#include <momemta/Types.h>
#include <momemta/Utils.h>

#include <Math/RotationZ.h>

/** \brief Flat transfer function on Phi (mainly for testing purposes). 
 *
 * This modules implements a constant (=1) transfer function on a particle's \f$\phi\f$.
 *
 * The primary purpose of this module is the validation of the phase-space generator, since it allows
 * computing phase-space volumes and cross-sections by integrating over the reconstructed particle's momenta.
 *
 * The module still takes a 4-momentum as input, since it needs an energy, a \f$\theta\f$-angle and a mass.
 *
 * The range of \f$\phi\f$ values considered is \f$[0,2\pi]\f$. 
 *
 * ### Integration dimension
 *
 * This module requires **1** phase-space point.
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
 *   | `output` | LorentzVector | Output *generated* LorentzVector, only differing from *reco_particle* by its \f$\phi\f$. |
 *   | `TF_times_jacobian` | double | Transfer function (ie, 1) times the jacobian (due to the integration range). |
 *
 * \ingroup modules
 */

class FlatTransferFunctionOnPhi: public Module {
    public:

        FlatTransferFunctionOnPhi(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            m_ps_point = get<double>(parameters.get<InputTag>("ps_point"));
            m_input = get<LorentzVector>(parameters.get<InputTag>("reco_particle"));
        };

        virtual Status work() override {

            const double& ps_point = *m_ps_point;
            const LorentzVector& reco_particle = *m_input;

            m_Rotation.SetAngle(2*M_PI*ps_point);

            *output = m_Rotation*reco_particle;
            
            // Compute TF*jacobian, ie the jacobian of the transformation of [0,1]->[0,2pi]
            *TF_times_jacobian = 2*M_PI;

            return Status::OK;
        }

    private:
        ROOT::Math::RotationZ m_Rotation;

        // Inputs
        Value<double> m_ps_point;
        Value<LorentzVector> m_input;

        // Outputs
        std::shared_ptr<LorentzVector> output = produce<LorentzVector>("output");
        std::shared_ptr<double> TF_times_jacobian = produce<double>("TF_times_jacobian");
};
REGISTER_MODULE(FlatTransferFunctionOnPhi);
