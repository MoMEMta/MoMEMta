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


#pragma once

#include <memory>
#include <vector>

#include <SharedLibrary.h>

class LibraryManager {
    public:
        static LibraryManager& get();

        void registerLibrary(const std::string& path);
    
    private:
        LibraryManager() = default;
        LibraryManager(const LibraryManager&) = delete;
        const LibraryManager& operator=(const LibraryManager&) = delete;

        std::vector<std::shared_ptr<SharedLibrary>> m_libraries;
};
