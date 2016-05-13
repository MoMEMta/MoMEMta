#pragma once

#include <complex> 
#include <memory> 

#include <momemta/MEParameters.h> 

namespace SLHA 
{
class Reader;}

  namespace example_WW_llvv 
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
      std::complex<double> GC_1, GC_2, GC_4, GC_50, GC_51, GC_53, GC_58,
          GC_100;
      // Model couplings dependent on aS

  }; 

  }

