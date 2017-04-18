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

// Conversion factor to go from pb to GeV^-2
#define CONVERSION 2.56819e-9

#define XS_WW_DILEP 0.866 // +- 0.005
#define XS_TT_DILEP 6.73 // +- 0.02

int main(int argc, char** argv) {

    logging::set_level(logging::level::debug);

    PSGenerationTestManager manager;

    // W(lnu)W(lnu)
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/crossSections/blockA_WW_dilep.lua", { "p1", "p2", "p3", "p4" }, { 0, 0, 0, 0 }, CONVERSION * XS_WW_DILEP }, 'A');
    // W(lnu)W(lnu)
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/crossSections/blockB_WW_dilep.lua", { "p1", "p2", "p3" }, { 0, 0, 0 }, CONVERSION * XS_WW_DILEP }, 'B');
    // W(lnu)W(lnu)
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/crossSections/blockB_secondaryBlockCD_WW_dilep.lua", { "p1", "p2", "p3" }, { 0, 0, 0 }, CONVERSION * XS_WW_DILEP }, 'B', 'C');
    // t(blnu)tx(blnu)
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/crossSections/blockD_ttx_dilep.lua", { "p1", "p2", "p3", "p4" }, { 0, 0, 0, 0 }, CONVERSION * XS_TT_DILEP }, 'D');
    // W(lnu)W(lnu)
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/crossSections/blockF_WW_dilep.lua", { "p1", "p2" }, { 0, 0 }, CONVERSION * XS_WW_DILEP }, 'F');
    // W(lnu)W(lnu)
    manager.registerTest({ SOURCE_PATH "/tests/phaseSpaceGeneration/crossSections/blockG_WW_dilep.lua", { "p1", "p2", "p3", "p4" }, { 0, 0, 0, 0 }, CONVERSION * XS_WW_DILEP }, 'G');

    manager.parseArgs(argc, argv);

    return 0;
}
