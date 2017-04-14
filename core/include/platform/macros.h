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

// Compiler attributes

// Use C++17 attributes if supported
#if __cplusplus >= 201703L

#define ATTRIBUTE_DEPRECATED [[deprecated]]
#define ATTRIBUTE_UNUSED [[maybe_unused]]

// Or C++14
#elif __cplusplus >= 201402L

#define ATTRIBUTE_DEPRECATED [[deprecated]]

#endif

#if (defined(__GNUC__) || defined(__APPLE__))

// Compiler supports GCC-style attributes
#ifndef ATTRIBUTE_DEPRECATED
#define ATTRIBUTE_DEPRECATED __attribute__((deprecated))
#endif

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED __attribute__((unused))
#endif

#else

#ifndef ATTRIBUTE_DEPRECATED
#define ATTRIBUTE_DEPRECATED
#endif

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

#endif