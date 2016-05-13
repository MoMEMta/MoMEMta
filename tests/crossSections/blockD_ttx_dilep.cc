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
#include <momemta/Utils.h>

#include <chrono>

using namespace std::chrono;

int main(int argc, char** argv) {

    UNUSED(argc);
    UNUSED(argv);

    ConfigurationReader configuration("../tests/crossSections/blockD_ttx_dilep.lua");
    MoMEMta weight(configuration.freeze());

    // Electron
    LorentzVector p3(-25, 0, 0, 25);
    // b-quark
    LorentzVector p4(-50, 0, 0, 50);
    // Muon
    LorentzVector p5(0, 50, 0, 50);
    // Anti b-quark
    LorentzVector p6(0, -25, 0, 25);

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
