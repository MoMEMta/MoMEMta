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

#include <momemta/Logging.h>

#include "phaseSpaceGenerationTests.h"

int main(int argc, char** argv) {

    logging::set_level(logging::level::debug);

    PSGenerationTestManager manager;

    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/phaseSpaceVolume/blockA.lua", { "p1", "p2", "p3" }, { 0, 0, 0 }, 6.3e-5 }, 'A');
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/phaseSpaceVolume/blockA_secondaryBlockCD.lua", { "p1", "p2", "p3", "p4" }, { 0, 0, 0, 0 }, 1.66e-2 }, 'A', 'C');
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/phaseSpaceVolume/blockB.lua", { "p1", "p2" }, { 0, 0 }, 6.3e-5 }, 'B');
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/phaseSpaceVolume/blockB_secondaryBlockA.lua", { "p2", "p3", "p4" }, { 0, 0, 0 }, 3.89 }, 'B', 'A');
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/phaseSpaceVolume/blockB_secondaryBlockB.lua", { "p1", "p2", "p3" }, { 0, 0, 0 }, 1.66e-2 }, 'B', 'B');
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/phaseSpaceVolume/blockB_secondaryBlockE.lua", { "p1", "p2", "p3" }, { 0, 0, 0 }, 1.66e-2 }, 'B', 'E');
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/phaseSpaceVolume/blockD.lua", { "p1", "p2", "p3", "p4" }, { 0, 0, 0, 0 }, 694 }, 'D');
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/phaseSpaceVolume/blockE.lua", { "p1", "p3" }, { 0, 0 }, 1.66e-2 }, 'E');
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/phaseSpaceVolume/blockF.lua", { "p1", "p2" }, { 0, 0 }, 1.66e-2 }, 'F');
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/phaseSpaceVolume/blockG.lua", { "p1", "p2", "p3", "p4" }, { 0, 0, 0, 0 }, 1.66e-2 }, 'G');

    manager.parseArgs(argc, argv);

    return 0;
}
