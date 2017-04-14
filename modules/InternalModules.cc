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

#include <momemta/ModuleRegistry.h>

#define REGISTER_INTERNAL_MODULE(name) \
    REGISTER_INTERNAL_MODULE_UNIQ_HELPER(__LINE__, "_" name)
#define REGISTER_INTERNAL_MODULE_UNIQ_HELPER(ctr, name) REGISTER_INTERNAL_MODULE_UNIQ(ctr, name)
#define REGISTER_INTERNAL_MODULE_UNIQ(ctr, name) \
  static const ::momemta::registration::ModuleDefBuilderReceiver register_module##ctr = \
          ::momemta::registration::ModuleDefBuilder(name)

REGISTER_INTERNAL_MODULE("cuba")
        .Output("ps_points")
        .Output("ps_weights");

REGISTER_INTERNAL_MODULE("met")
        .Output("p4");

REGISTER_INTERNAL_MODULE("input")
        .Output("p4")
        .Output("type");

REGISTER_INTERNAL_MODULE("momemta")
        .Inputs("integrands");