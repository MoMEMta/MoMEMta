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

/**
 * \file
 * \brief No integration integration test
 * \ingroup IntegrationTests
 */

#include <catch.hpp>

#include <momemta/ConfigurationReader.h>
#include <momemta/Logging.h>
#include <momemta/MoMEMta.h>

TEST_CASE("No integration", "[integration_tests]") {
    logging::set_level(logging::level::fatal);

    ConfigurationReader configuration("no_integration.lua");
    MoMEMta weight(configuration.freeze());

    // Electron
    LorentzVector p3(16.171895980835, -13.7919054031372, -3.42997527122497, 21.5293197631836);
    // b-quark
    LorentzVector p4(-55.7908325195313, -111.59294128418, -122.144721984863, 174.66259765625);
    // Muon
    LorentzVector p5(-18.9018573760986, 10.0896110534668, -0.602926552295686, 21.4346446990967);
    // Anti b-quark
    LorentzVector p6(71.3899612426758, 96.0094833374023, -77.2513122558594, 142.492813110352);
    // Electronic neutrino
    LorentzVector p1(-57.9413, 40.7629, -54.2982, 89.2587);
    // Muonic neutrino
    LorentzVector p2(57.9413, -40.7629, -40.8437, 81.7742);

    std::vector<std::pair<double, double>> weights = weight.computeWeights({p3, p4, p5, p6, p1, p2});

    REQUIRE(weight.getIntegrationStatus() == MoMEMta::IntegrationStatus::SUCCESS);

    REQUIRE(weights.size() == 1);
    REQUIRE(weights[0].first == Approx(8.36916e-13));
    REQUIRE(weights[0].second == Approx(0.));
}
