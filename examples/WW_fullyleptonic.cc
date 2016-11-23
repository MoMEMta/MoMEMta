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


#include <momemta/ConfigurationReader.h>
#include <momemta/MoMEMta.h>
#include <momemta/Unused.h>

#include <TH1D.h>

#include <chrono>

using namespace std::chrono;
using namespace momemta;

int main(int argc, char** argv) {

    UNUSED(argc);
    UNUSED(argv);

    logging::set_level(logging::level::debug);

    ConfigurationReader configuration("../examples/WW_fullyleptonic.lua");

    MoMEMta weight(configuration.freeze());

    // Electron
    Particle electron { "electron", LorentzVector(16.171895980835, -13.7919054031372, -3.42997527122497, 21.5293197631836), -11 };
    // Muon
    Particle muon { "muon", LorentzVector(-18.9018573760986, 10.0896110534668, -0.602926552295686, 21.4346446990967), +13 };

    auto start_time = system_clock::now();
    std::vector<std::pair<double, double>> weights = weight.computeWeights({electron, muon});
    auto end_time = system_clock::now();

    LOG(debug) << "Result:";
    for (const auto& r: weights) {
        LOG(debug) << r.first << " +- " << r.second;
    }

    LOG(info) << "Weight computed in " << std::chrono::duration_cast<milliseconds>(end_time - start_time).count() << "ms";


    return 0;
}
