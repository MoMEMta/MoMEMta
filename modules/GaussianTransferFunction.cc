#include <Math/DistFunc.h>

#include <ConfigurationSet.h>
#include <Module.h>
#include <Types.h>
#include <Utils.h>
#include <logging.h>

class GaussianTransferFunction: public Module {
    public:

        GaussianTransferFunction(PoolPtr pool, const ConfigurationSet& parameters): Module(pool, parameters.getModuleName()) {
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
            *jacobian = ROOT::Math::normal_pdf(gen_E, reco_particle.E(), sigma) * range * dE_over_dP(*output);
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
        std::shared_ptr<double> jacobian = produce<double>("jacobian");

};
REGISTER_MODULE(GaussianTransferFunction);
