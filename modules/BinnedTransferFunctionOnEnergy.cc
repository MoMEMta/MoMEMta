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


#include <TFile.h>
#include <TH2.h>
#include <TAxis.h>

/** \brief Helper class for binned transfer function modules
 *
 * Base class helping to binned define TF modules having different behaviours (allowing either to integrate over a TF, or simply evaluate it).
 * The class handles opening the TFile, loading the histograms, computing the ranges, ...
 *
 * \sa BinnedTransferFunctionOnEnergy
 * \sa BinnedTransferFunctionOnEnergyEvaluator
 */
class BinnedTransferFunctionOnEnergyBase: public Module {
    public:

        BinnedTransferFunctionOnEnergyBase(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            m_reco_input = get<LorentzVector>(parameters.get<InputTag>("reco_particle"));

            m_file_path = parameters.get<std::string>("file");
            m_th2_name = parameters.get<std::string>("th2_name");

            m_file = std::move( std::unique_ptr<TFile>( TFile::Open(m_file_path.c_str()) ) );
            if(!m_file->IsOpen() || m_file->IsZombie())
                throw file_not_found_error("Could not open file " + m_file_path);

            m_th2 = std::move( std::unique_ptr<TH2>( static_cast<TH2*>( m_file->Get(m_th2_name.c_str()) ) ) );
            if(!m_th2->InheritsFrom("TH2") || !m_th2.get())
                throw th2_not_found_error("Could not retrieve object " + m_th2_name + " deriving from class TH2 in file " + m_file_path + ".");
            m_th2->SetDirectory(0);

            TAxis* yAxis = m_th2->GetYaxis();
            m_deltaMin = yAxis->GetXmin();
            m_deltaMax = yAxis->GetXmax();
            m_deltaRange = m_deltaMax - m_deltaMin;
            
            TAxis* xAxis = m_th2->GetXaxis();
            m_EgenMin = xAxis->GetXmin();
            m_EgenMax = xAxis->GetXmax();

            // Since we assume the TF continues as a constant for E->infty,
            // we need to be able to retrieve the X axis' last bin, to avoid
            // fetching the TH2's overflow bin.
            m_fallBackEgenMax = xAxis->GetBinCenter(xAxis->GetNbins());
            
            LOG(debug) << "Loaded TH2 " << m_th2_name << " from file " << m_file_path << ".";
            LOG(debug) << "\tDelta range is " << m_deltaMin << " to " << m_deltaMax << ".";
            LOG(debug) << "\tEnergy range is " << m_EgenMin << " to " << m_EgenMax << ".";
            LOG(debug) << "\tWill use values at Egen = " << m_fallBackEgenMax << " for out-of-range values.";

            m_file->Close();
            m_file.reset();
        };

    protected:
        std::unique_ptr<TH2> m_th2;

        double m_deltaMin, m_deltaMax, m_deltaRange;
        double m_EgenMin, m_EgenMax;
        double m_fallBackEgenMax;

        // Input
        Value<LorentzVector> m_reco_input;

    private:
        std::string m_file_path;
        std::string m_th2_name;

        std::unique_ptr<TFile> m_file;

        class file_not_found_error: public std::runtime_error{
            using std::runtime_error::runtime_error;
        };
        
        class th2_not_found_error: public std::runtime_error{
            using std::runtime_error::runtime_error;
        };
};

/** \brief Integrate over a transfer function on energy described by a 2D histogram retrieved from a ROOT file.
 * 
 * This module takes as input a LorentzVector and a phase-space point, generates
 * a new LorentzVector with a different energy (keeping direction and invariant mass),
 * and evaluates the transfer function on the "reconstructed" and "generated" energies.
 *
 * The transfer function (TF) is described by a 2D histogram (a ROOT TH2), where the X-axis defines the "generated" (true) energy \f$E_{gen}\f$,
 * and the Y-axis the difference between the reconstructed and the generated energy (\f$E_{rec}-E_{gen}\f$).
 *
 * It is assumed the TF is correctly normalised, i.e. integrals of slices along the Y-axis sum up to 1.
 *
 * The integration is done over the whole width of the TF. The TF is assumed to continue asymptotically as a
 * constant for \f$E_{gen} \to \infty\f$.
 *
 * ### Integration dimension
 *
 * This module requires **1** phase-space point.
 *
 * ### Parameters
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `file` | string | Path to the ROOT file in which the transfer function is saved. |
 *   | `th2_name` | string | Name of the TH2 stored in file `file` |
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
 *   | `output` | LorentzVector | Output *generated* LorentzVector, only differing from *reco_particle* by its energy. |
 *   | `TF_times_jacobian` | double | Product of the TF evaluated on the *reco* and *gen* energies, times the jacobian of the transformation needed stretch the integration range from \f$[0,1]\f$ to the width of the TF, times the jacobian \f$dE/d|P|\f$ due to the fact that the integration is done w.r.t \f$|P|\f$, while the TF is parametrised in terms of energy. |
 *
 * \ingroup modules
 * \sa BinnedTransferFunctionOnEnergyEvaluator
 */
class BinnedTransferFunctionOnEnergy: public BinnedTransferFunctionOnEnergyBase {
    public:

        BinnedTransferFunctionOnEnergy(PoolPtr pool, const ParameterSet& parameters): BinnedTransferFunctionOnEnergyBase(pool, parameters) {
            m_ps_point = get<double>(parameters.get<InputTag>("ps_point"));
        }
        
        virtual Status work() override {
            const double rec_E = m_reco_input->E();
            const double range = GetDeltaRange(rec_E);
            const double gen_E = rec_E - GetDeltaMax(rec_E) + range * (*m_ps_point);
            const double delta = rec_E - gen_E;

            // To change the particle's energy without changing its direction and mass:
            const double gen_pt = std::sqrt(SQ(gen_E) - SQ(m_reco_input->M())) / std::cosh(m_reco_input->Eta());
            output->SetCoordinates(
                    gen_pt * std::cos(m_reco_input->Phi()),
                    gen_pt * std::sin(m_reco_input->Phi()),
                    gen_pt * std::sinh(m_reco_input->Eta()),
                    gen_E);

            // The bin number is a ROOT "global bin number" using a 1D representation of the TH2
            const int bin = m_th2->FindFixBin(std::min(gen_E, m_fallBackEgenMax), delta);
            
            // Compute TF*jacobian, where the jacobian includes the transformation of [0,1]->[range_min,range_max] and d|P|/dE
            *TF_times_jacobian = m_th2->GetBinContent(bin) * range * dP_over_dE(*output);

            return Status::OK;
        }

    private:

        // Input
        Value<double> m_ps_point;
        
        // Outputs
        std::shared_ptr<LorentzVector> output = produce<LorentzVector>("output");
        std::shared_ptr<double> TF_times_jacobian = produce<double>("TF_times_jacobian");

        // We assume TF=0 for Egen < lower limit of the TH2, and
        //           TF=constant for Egen > upper limit of the TH2
        // In the former case, the integrated range is adapted (shortened) to the range where TF!=0
        inline double GetDeltaRange(const double Erec) const {
            return GetDeltaMax(Erec) - m_deltaMin;
        }

        inline double GetDeltaMax(const double Erec) const {
            return std::min(m_deltaMax, Erec - m_EgenMin); 
        }
};

/** \brief Evaluate a transfer function on energy described by a 2D histogram retrieved from a ROOT file.
 * 
 * This module takes as inputs two LorentzVectors: a 'gen-level' particle (which may be computed using for instance a Block or a 'real' transfer function) and a 'reco-level' particle (experimentally reconstructed). 
 * Assuming the LorentzVectors differ only by their energy, this module returns the value of a transfer function (TF) evaluated on their respective energies.
 *
 * The TF is described by a 2D histogram (a ROOT TH2), where the X-axis defines the "generated" (true) energy \f$E_{gen}\f$,
 * and the Y-axis the difference between the reconstructed and the generated energy (\f$E_{rec}-E_{gen}\f$).
 *
 * It is assumed the histogram is correctly normalised, i.e. integrals of slices along the Y-axis sum up to 1. The TF is assumed to continue asymptotically as a constant for \f$E_{gen} \to \infty\f$. If the difference \f$E_{rec}-E_{gen}\f$ is outside the TH2's Y-axis range, the TF evaluates to 0.
 *
 * ### Integration dimension
 *
 * This module requires **0** phase-space points.
 *
 * ### Parameters
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `file` | string | Path to the ROOT file in which the transfer function is saved. |
 *   | `th2_name` | string | Name of the TH2 stored in file `file` |
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
 *   | `TF` | double | Value of the TF evaluated on the *reco* and *gen* energies. |
 *
 * \ingroup modules
 * \sa BinnedTransferFunctionOnEnergyEvaluator
 */
class BinnedTransferFunctionOnEnergyEvaluator: public BinnedTransferFunctionOnEnergyBase {
    public:

        BinnedTransferFunctionOnEnergyEvaluator(PoolPtr pool, const ParameterSet& parameters): BinnedTransferFunctionOnEnergyBase(pool, parameters) {
            m_gen_input = get<LorentzVector>(parameters.get<InputTag>("gen_particle"));
        }
        
        virtual Status work() override {
            const double rec_E = m_reco_input->E();
            const double gen_E = m_gen_input->E();
            double delta = rec_E - gen_E;
            if (delta <= m_deltaMin || delta >= m_deltaMax) {
                *TF_value = 0;
                return Status::OK;
            }

            // The bin number is a ROOT "global bin number" using a 1D representation of the TH2
            const int bin = m_th2->FindFixBin(std::min(gen_E, m_fallBackEgenMax), delta);
            
            // Retrieve TF value
            *TF_value = m_th2->GetBinContent(bin);

            return Status::OK;
        }

    private:

        // Input
        Value<LorentzVector> m_gen_input;
        
        // Outputs
        std::shared_ptr<double> TF_value = produce<double>("TF");
};

REGISTER_MODULE(BinnedTransferFunctionOnEnergy);
REGISTER_MODULE(BinnedTransferFunctionOnEnergyEvaluator);
