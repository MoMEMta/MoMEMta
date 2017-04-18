/*
 *  MoMEMta: a modular implementation of the Matrix Element Method
 *  Copyright (C) 2017  Universite catholique de Louvain (UCL), Belgium
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
#include <momemta/Types.h>
#include <momemta/Utils.h>

#include "phaseSpaceGenerationTests.h"
    
#include <chrono>
#include <exception>

using namespace std::chrono;
using namespace momemta;

void PSGenerationTestManager::registerTest(PSGenerationTest test, char mainBlock, char secondaryBlock/*='\0'*/) {
    assert(test.particle_names.size() == test.particle_masses.size());

    if (secondaryBlock == '\0')
        mainBlocks.emplace(mainBlock, test);
    else
        secondaryBlocks.emplace(std::make_pair(mainBlock, secondaryBlock), test);
}

void PSGenerationTestManager::parseArgs(int argc, char** argv) {
    if (argc <= 1) {
        printHelpMessage();
        throw std::invalid_argument("Expected at least one argument!");
    }

    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);

        if (arg == "-h" || arg == "--help") {
            printHelpMessage();
            break;
        }

        if (arg.length() == 1)
            runTest(arg[0]);
        else if (arg.length() == 2)
            runTest(arg[0], arg[1]);
        else {
            printHelpMessage();
            throw std::invalid_argument("Each argument has to be one or two characters!");
        }
    }
}

void PSGenerationTestManager::printHelpMessage() {
    LOG(info) << "Usage: X[Y] [X[Y] ...]";
    LOG(info) << "\t where X and Y are characters indicating which main [optionally in combination with a secondary] block is requested.";
    LOG(info) << "Available block combinations:";
    LOG(info) << "\tMain Blocks:";
    for (auto it: mainBlocks)
        LOG(info) << "\t\t" << it.first;
    LOG(info) << "\tMain Blocks with Secondary Blocks:";
    for (auto it: secondaryBlocks)
        LOG(info) << "\t\t" << it.first.first << " (main) combined with " << it.first.second << " (secondary)";
}

void PSGenerationTestManager::runTest(char mainBlock, char secondaryBlock/*='\0'*/) {
    if (secondaryBlock == '\0') {
        LOG(debug) << "Attempting to run test for Main Block " << mainBlock;
        auto it = mainBlocks.find(mainBlock);
        if (it == mainBlocks.end()) {
            printHelpMessage();
            throw std::invalid_argument("Could not find test for main block!");
        }
        runTest(it->second);
    } else {
        LOG(debug) << "Attempting to run test for combination of Main Block " << mainBlock << " and Secondary Block " << secondaryBlock;
        auto it = secondaryBlocks.find(std::make_pair(mainBlock, secondaryBlock));
        if (it == secondaryBlocks.end()) {
            printHelpMessage();
            throw std::invalid_argument("Could not find test for main and secondary block combination!");
        }
        runTest(it->second);
    }
}

void PSGenerationTestManager::runTest(PSGenerationTest &test) {
    ConfigurationReader configuration(test.lua_file);
    MoMEMta weight(configuration.freeze());

    std::vector<Particle> particles;
    for (std::size_t i = 0; i < test.particle_names.size(); i++)
        particles.push_back(Particle(test.particle_names[i], getRandom4Vector(100, test.particle_masses[i]), 0));

    auto start_time = system_clock::now();
    std::vector<std::pair<double, double>> weights = weight.computeWeights(particles);
    auto end_time = system_clock::now();

    LOG(info) << "Result: " << weights.back().first << " +- " << weights.back().second;
    LOG(info) << "Target value: " << test.target_value;
    LOG(info) << "Ratio: " << std::fixed << std::setprecision(4) << weights.back().first / test.target_value << " +- " << weights.back().second / test.target_value;

    LOG(info) << "Weight computed in " << std::chrono::duration_cast<milliseconds>(end_time - start_time).count() << "ms";
}
