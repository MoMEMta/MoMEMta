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

#pragma once

#include <utility>
#include <vector>
#include <map>

struct PSGenerationTest {
    std::string lua_file;
    std::vector<std::string> particle_names;
    std::vector<double> particle_masses;
    double target_value;
};

class PSGenerationTestManager {
    public:

        void registerTest(PSGenerationTest test, char mainBlock, char secondaryBlock='\0');
        void parseArgs(int argc, char** argv);
        void runTest(char mainBlock, char secondaryBlock='\0');

    private:

        void runTest(PSGenerationTest &test);
        void printHelpMessage();

        std::map<char, PSGenerationTest> mainBlocks;
        std::map<std::pair<char, char>, PSGenerationTest> secondaryBlocks;
};
