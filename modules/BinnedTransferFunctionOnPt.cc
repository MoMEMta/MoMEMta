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
 * \sa BinnedTransferFunctionOnPt
 * \sa BinnedTransferFunctionOnPtEvaluator
 */
class BinnedTransferFunctionOnPtBase: public Module {
    public:

        BinnedTransferFunctionOnPtBase(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {
            m_reco_input = get<LorentzVector>(parameters.get<InputTag>("reco_particle"));

            std::string file_path = parameters.get<std::string>("file");
            std::string th2_name = parameters.get<std::string>("th2_name");

            std::unique_ptr<TFile> file(TFile::Open(file_path.c_str()));
            if(!file->IsOpen() || file->IsZombie())
                throw file_not_found_error("Could not open file " + file_path);

            m_th2 = std::unique_ptr<TH2>(static_cast<TH2*>(file->Get(th2_name.c_str())));
            if(!m_th2->InheritsFrom("TH2") || !m_th2.get())
                throw th2_not_found_error("Could not retrieve object " + th2_name + " deriving from class TH2 in file " + file_path + ".");
            m_th2->SetDirectory(0);

            TAxis* yAxis = m_th2->GetYaxis();
            m_deltaMin = yAxis->GetXmin();
            m_deltaMax = yAxis->GetXmax();
            m_deltaRange = m_deltaMax - m_deltaMin;

            TAxis* xAxis = m_th2->GetXaxis();
            double Pt_cut = parameters.get<double>("min_Pt", 0.);
            m_PtgenMin = std::max(xAxis->GetXmin(), Pt_cut);
            m_PtgenMax = xAxis->GetXmax();

            // Since we assume the TF continues as a constant for Pt->infty,
            // we need to be able to retrieve the X axis' last bin, to avoid
            // fetching the TH2's overflow bin.
            m_fallBackPtgenMax = xAxis->GetBinCenter(xAxis->GetNbins());

            LOG(debug) << "Loaded TH2 " << th2_name << " from file " << file_path << ".";
            LOG(debug) << "\tDelta range is " << m_deltaMin << " to " << m_deltaMax << ".";
            LOG(debug) << "\tPt range is " << m_PtgenMin << " to " << m_PtgenMax << ".";
            LOG(debug) << "\tWill use values at Ptgen = " << m_fallBackPtgenMax << " for out-of-range values.";

            file->Close();
            file.reset();
        };

    protected:
        std::unique_ptr<TH2> m_th2;

        double m_deltaMin, m_deltaMax, m_deltaRange;
        double m_PtgenMin, m_PtgenMax;
        double m_fallBackPtgenMax;

        // Input
        Value<LorentzVector> m_reco_input;

    private:
        class file_not_found_error: public std::runtime_error{
            using std::runtime_error::runtime_error;
        };

        class th2_not_found_error: public std::runtime_error{
            using std::runtime_error::runtime_error;
        };
};

/** \brief Integrate over a transfer function on Pt described by a 2D histogram retrieved from a ROOT file.
 *
 * This module takes as input a LorentzVector and a phase-space point, generates
 * a new LorentzVector with a different Pt (keeping direction and invariant mass),
 * and evaluates the transfer function on the "reconstructed" and "generated" Pt.
 *
 * The transfer function (TF) is described by a 2D histogram (a ROOT TH2), where the X-axis defines the "generated" (true) Pt \f$P_T_{gen}\f$,
 * and the Y-axis the difference between the reconstructed and the generated Pt (\f$P_T_{rec}-P_T_{gen}\f$).
 *
 * It is assumed the TF is correctly normalised, i.e. integrals of slices along the Y-axis sum up to 1.
 *
 * The integration is done over the whole width of the TF. The TF is assumed to continue asymptotically as a
 * constant for \f$P_T_{gen} \to \infty\f$.
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
 *   | `TF_times_jacobian` | double | Product of the TF evaluated on the *reco* and *gen* Pt, times the jacobian of the transformation needed stretch the integration range from \f$[0,1]\f$ to the width of the TF, times the jacobian \f$d|P|/dP_T=cosh \eta\f$ due to the fact that the integration is done w.r.t \f$|P|\f$, while the TF is parametrised in terms of Pt. |
 *
 * \ingroup modules
 * \sa BinnedTransferFunctionOnPtEvaluator
 */
class BinnedTransferFunctionOnPt: public BinnedTransferFunctionOnPtBase {
    public:

        BinnedTransferFunctionOnPt(PoolPtr pool, const ParameterSet& parameters): BinnedTransferFunctionOnPtBase(pool, parameters) {
            m_ps_point = get<double>(parameters.get<InputTag>("ps_point"));
        }

        virtual Status work() override {
            const double rec_Pt = m_reco_input->Pt();
            const double cosh_eta = std::cosh(m_reco_input->Eta());
            const double range = GetDeltaRange(rec_Pt);
            const double gen_Pt = rec_Pt - GetDeltaMax(rec_Pt) + range * (*m_ps_point);
            const double delta = rec_Pt - gen_Pt;

            // To change the particle's Pt without changing its direction and mass:
            const double gen_E = std::sqrt(SQ(m_reco_input->M()) + SQ(cosh_eta * gen_Pt));
            output->SetCoordinates(
                    gen_Pt * std::cos(m_reco_input->Phi()),
                    gen_Pt * std::sin(m_reco_input->Phi()),
                    gen_Pt * std::sinh(m_reco_input->Eta()),
                    gen_E);

            // The bin number is a ROOT "global bin number" using a 1D representation of the TH2
            const int bin = m_th2->FindFixBin(std::min(gen_Pt, m_fallBackPtgenMax), delta);

            // Compute TF*jacobian, where the jacobian includes the transformation of [0,1]->[range_min,range_max] and d|P|/dP_T = cosh(eta)
            *TF_times_jacobian = m_th2->GetBinContent(bin) * range * cosh_eta;

            return Status::OK;
        }

    private:

        // Input
        Value<double> m_ps_point;

        // Outputs
        std::shared_ptr<LorentzVector> output = produce<LorentzVector>("output");
        std::shared_ptr<double> TF_times_jacobian = produce<double>("TF_times_jacobian");

        // We assume TF=0 for Ptgen < lower limit of the TH2, and
        //           TF=constant for Ptgen > upper limit of the TH2
        // In the former case, the integrated range is adapted (shortened) to the range where TF!=0
        inline double GetDeltaRange(const double Ptrec) const {
            return GetDeltaMax(Ptrec) - m_deltaMin;
        }

        inline double GetDeltaMax(const double Ptrec) const {
            return std::min(m_deltaMax, Ptrec - m_PtgenMin);
        }
};

/** \brief Evaluate a transfer function on Pt described by a 2D histogram retrieved from a ROOT file.
 *
 * This module takes as inputs two LorentzVectors: a 'gen-level' particle (which may be computed using for instance a Block or a 'real' transfer function) and a 'reco-level' particle (experimentally reconstructed).
 * Assuming the LorentzVectors differ only by their Pt, this module returns the value of a transfer function (TF) evaluated on their respective Pt.
 *
 * The transfer function (TF) is described by a 2D histogram (a ROOT TH2), where the X-axis defines the "generated" (true) Pt \f$P_T_{gen}\f$,
 * and the Y-axis the difference between the reconstructed and the generated Pt (\f$P_T_{rec}-P_T_{gen}\f$).
 *
 * It is assumed the TF is correctly normalised, i.e. integrals of slices along the Y-axis sum up to 1.
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
 *   | `TF` | double | Value of the TF evaluated on the *reco* and *gen* Pt. |
 *
 * \ingroup modules
 * \sa BinnedTransferFunctionOnPtEvaluator
 */
class BinnedTransferFunctionOnPtEvaluator: public BinnedTransferFunctionOnPtBase {
    public:

        BinnedTransferFunctionOnPtEvaluator(PoolPtr pool, const ParameterSet& parameters): BinnedTransferFunctionOnPtBase(pool, parameters) {
            m_gen_input = get<LorentzVector>(parameters.get<InputTag>("gen_particle"));
        }

        virtual Status work() override {
            const double rec_Pt = m_reco_input->Pt();
            const double gen_Pt = m_gen_input->Pt();
            double delta = rec_Pt - gen_Pt;
            if (delta <= m_deltaMin || delta >= m_deltaMax) {
                *TF_value = 0;
                return Status::OK;
            }

            // The bin number is a ROOT "global bin number" using a 1D representation of the TH2
            const int bin = m_th2->FindFixBin(std::min(gen_Pt, m_fallBackPtgenMax), delta);

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

REGISTER_MODULE(BinnedTransferFunctionOnPt)
        .Input("reco_particle")
        .Input("ps_point")
        .Output("output")
        .Output("TF_times_jacobian")
        .Attr("file:string")
        .Attr("th2_name:string")
        .Attr("min_Pt:double=0");

REGISTER_MODULE(BinnedTransferFunctionOnPtEvaluator)
        .Input("reco_particle")
        .Input("gen_particle")
        .Output("output")
        .Output("TF_times_jacobian")
        .Attr("file:string")
        .Attr("th2_name:string")
        .Attr("min_Pt:double=0");