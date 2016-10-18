// 
// *  This file was automatically generated by MoMEMta-MaGMEE,
// *  A MadGraph Matrix Element Exporter plugin for MoMEMta.
// *
// *  It is subject to MoMEMta-MaGMEE's license and copyright:
// *
// *  Copyright (C) 2016  Universite catholique de Louvain (UCL), Belgium
// *
// *  This program is free software: you can redistribute it and/or modify
// *  it under the terms of the GNU General Public License as published by
// *  the Free Software Foundation, either version 3 of the License, or
// *  (at your option) any later version.
// *
// *  This program is distributed in the hope that it will be useful,
// *  but WITHOUT ANY WARRANTY; without even the implied warranty of
// *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// *  GNU General Public License for more details.
// *
// *  You should have received a copy of the GNU General Public License
// *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

#pragma once

#include <complex> 
#include <memory> 

#include <momemta/MEParameters.h> 

namespace SLHA 
{
class Reader;}

  namespace pp_zh_z_ee_h_bb_sm 
  {

  class Parameters_sm: public momemta::MEParameters
  {

    public:

      Parameters_sm(const SLHA::Reader& card); 

      // *
      // * Cache parameters. This is implementation dependent
      // 
      virtual void cacheParameters(); 
      virtual void cacheCouplings(); 

      virtual void updateParameters(); 
      virtual void updateCouplings(); 

      // Define "zero"
      double zero, ZERO; 
      // Model parameters independent of aS
      double mdl_WH, mdl_WW, mdl_WZ, mdl_WT, mdl_ymtau, mdl_ymt, mdl_ymb, aS,
          mdl_Gf, aEWM1, mdl_MH, mdl_MZ, mdl_MTA, mdl_MT, mdl_MB,
          mdl_conjg__CKM3x3, mdl_CKM3x3, mdl_conjg__CKM1x1, mdl_MZ__exp__2,
          mdl_MZ__exp__4, mdl_sqrt__2, mdl_MH__exp__2, mdl_aEW, mdl_MW,
          mdl_sqrt__aEW, mdl_ee, mdl_MW__exp__2, mdl_sw2, mdl_cw,
          mdl_sqrt__sw2, mdl_sw, mdl_g1, mdl_gw, mdl_vev, mdl_vev__exp__2,
          mdl_lam, mdl_yb, mdl_yt, mdl_ytau, mdl_muH, mdl_ee__exp__2,
          mdl_sw__exp__2, mdl_cw__exp__2;
      std::complex<double> mdl_complexi, mdl_I1x33, mdl_I2x33, mdl_I3x33,
          mdl_I4x33;
      // Model parameters dependent on aS
      double mdl_sqrt__aS, G, mdl_G__exp__2; 
      // Model couplings independent of aS
      std::complex<double> GC_50, GC_51, GC_58, GC_59, GC_81, GC_83; 
      // Model couplings dependent on aS

  }; 

  }

