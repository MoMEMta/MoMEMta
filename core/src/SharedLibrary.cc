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


#include <dlfcn.h>

#include <logging.h>
#include <SharedLibrary.h>

SharedLibrary::SharedLibrary(const std::string& path) {
    m_handle = ::dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (! m_handle) {
        LOG(error) << "Failed to open '" << path << "': " << dlerror();
    }
}

SharedLibrary::~SharedLibrary() {
    if (m_handle != nullptr)
        ::dlclose(m_handle);
}
