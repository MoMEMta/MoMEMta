#include <ConfigurationReader.h>
#include <MEMpp.h>
#include <Types.h>
#include <logging.h>
#include <chrono>

using namespace std::chrono;

int main(int argc, char** argv) {

    spdlog::set_level(spdlog::level::trace);

    ConfigurationReader configuration("../confs/example.lua");
    MEMpp weight(configuration);

    // Electron
    LorentzVector p3(16.171895980835, -13.7919054031372, -3.42997527122497, 21.5293197631836);
    // b-quark
    LorentzVector p4(-55.7908325195313, -111.59294128418, -122.144721984863, 174.66259765625);
    // Muon
    LorentzVector p5(-18.9018573760986, 10.0896110534668, -0.602926552295686, 21.4346446990967);
    // Anti b-quark
    LorentzVector p6(71.3899612426758, 96.0094833374023, -77.2513122558594, 142.492813110352);

    auto start_time = system_clock::now();
    std::vector<std::pair<double, double>> weights = weight.computeWeights({p3, p4, p5, p6});
    auto end_time = system_clock::now();

    LOG(debug) << "Result:";
    for (const auto& r: weights) {
        LOG(debug) << r.first << " +- " << r.second;
    }

    LOG(info) << "Weight computed in " << std::chrono::duration_cast<milliseconds>(end_time - start_time).count() << "ms";


    return 0;
}
