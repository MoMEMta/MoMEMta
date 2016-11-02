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

using namespace momemta;

TEST_CASE("No integration", "[integration_tests]") {
    logging::set_level(logging::level::fatal);

    ConfigurationReader configuration("no_integration.lua");
    MoMEMta weight(configuration.freeze());

    // Electron
    Particle electron { "electron", LorentzVector(16.171895980835, -13.7919054031372, -3.42997527122497, 21.5293197631836), -11 };
    // b-quark
    Particle bjet1 { "bjet1", LorentzVector(-55.7908325195313, -111.59294128418, -122.144721984863, 174.66259765625), 5 };
    // Muon
    Particle muon { "muon", LorentzVector(-18.9018573760986, 10.0896110534668, -0.602926552295686, 21.4346446990967), +13 };
    // Anti b-quark
    Particle bjet2 { "bjet2", LorentzVector(71.3899612426758, 96.0094833374023, -77.2513122558594, 142.492813110352), -5 };
    // Electronic neutrino
    Particle nu1 { "neutrino1", LorentzVector(-57.9413, 40.7629, -54.2982, 89.2587), +12 };
    // Muonic neutrino
    Particle nu2 { "neutrino2", LorentzVector(57.9413, -40.7629, -40.8437, 81.7742), -14 };

    std::vector<std::pair<double, double>> weights = weight.computeWeights({electron, muon, bjet1, bjet2, nu1, nu2});

    REQUIRE(weight.getIntegrationStatus() == MoMEMta::IntegrationStatus::SUCCESS);

    REQUIRE(weights.size() == 1);
    REQUIRE(weights[0].first == Approx(8.36916e-13));
    REQUIRE(weights[0].second == Approx(0.));
}
