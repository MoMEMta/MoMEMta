//==========================================================================
// This file has been automatically generated for C++ Standalone by
// MadGraph5_aMC@NLO v. 2.3.3, 2015-10-25
// By the MadGraph5_aMC@NLO Development Team
// Visit launchpad.net/madgraph5 and amcatnlo.web.cern.ch
//==========================================================================

#ifndef MG5_Sigma_sm_gg_mupvmbmumvmxbx_H
#define MG5_Sigma_sm_gg_mupvmbmumvmxbx_H

#include <complex> 
#include <vector> 
#include <utility> 
#include <map> 

#include "../../src/Parameters_sm.h"
#include "../../src/process_base_classes.h"

//==========================================================================
// A class for calculating the matrix elements for
// Process: g g > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > mu+ vm WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > mu- vm~ WEIGHTED=2
// Process: u u~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > mu+ vm WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > mu- vm~ WEIGHTED=2
// Process: c c~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > mu+ vm WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > mu- vm~ WEIGHTED=2
// Process: d d~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > mu+ vm WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > mu- vm~ WEIGHTED=2
// Process: s s~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > mu+ vm WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > mu- vm~ WEIGHTED=2
// Process: g g > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > e+ ve WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > mu- vm~ WEIGHTED=2
// Process: u u~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > e+ ve WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > mu- vm~ WEIGHTED=2
// Process: c c~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > e+ ve WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > mu- vm~ WEIGHTED=2
// Process: d d~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > e+ ve WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > mu- vm~ WEIGHTED=2
// Process: s s~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > e+ ve WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > mu- vm~ WEIGHTED=2
// Process: g g > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > mu+ vm WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > e- ve~ WEIGHTED=2
// Process: u u~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > mu+ vm WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > e- ve~ WEIGHTED=2
// Process: c c~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > mu+ vm WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > e- ve~ WEIGHTED=2
// Process: d d~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > mu+ vm WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > e- ve~ WEIGHTED=2
// Process: s s~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > mu+ vm WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > e- ve~ WEIGHTED=2
// Process: g g > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > e+ ve WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > e- ve~ WEIGHTED=2
// Process: u u~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > e+ ve WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > e- ve~ WEIGHTED=2
// Process: c c~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > e+ ve WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > e- ve~ WEIGHTED=2
// Process: d d~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > e+ ve WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > e- ve~ WEIGHTED=2
// Process: s s~ > t t~ WEIGHTED=2 @1
// *   Decay: t > w+ b WEIGHTED=2
// *     Decay: w+ > e+ ve WEIGHTED=2
// *   Decay: t~ > w- b~ WEIGHTED=2
// *     Decay: w- > e- ve~ WEIGHTED=2
//--------------------------------------------------------------------------

// Forward declaration needed to template correctly __MatrixElement in the class
class cpp_pp_ttx_fullylept; 

  class cpp_pp_ttx_fullylept: public CPPProcess
  {
    public:

      // Constructor & destructor
      cpp_pp_ttx_fullylept(Parameters_sm &params); 
      virtual ~cpp_pp_ttx_fullylept() {}; 

      // Calculate flavour-independent parts of cross section.
      virtual std::map < std::pair < int, int > , double > sigmaKin(const
          std::vector < std::vector<double> > &initialMomenta, const
          std::vector < std::pair < int, std::vector<double> > > &finalState);

      // Info on the subprocess.
      virtual std::string name() const {return "g g > mu+ vm b mu- vm~ b~ (sm)";}

      const std::vector<double> & getMasses() const {return mME;} 

    private:

      // default constructor should be hidden
      cpp_pp_ttx_fullylept(); 

      // list of helicities combinations
      const int helicities[256][8] = {{-1, -1, -1, -1, -1, -1, -1, -1}, {-1,
          -1, -1, -1, -1, -1, -1, 1}, {-1, -1, -1, -1, -1, -1, 1, -1}, {-1, -1,
          -1, -1, -1, -1, 1, 1}, {-1, -1, -1, -1, -1, 1, -1, -1}, {-1, -1, -1,
          -1, -1, 1, -1, 1}, {-1, -1, -1, -1, -1, 1, 1, -1}, {-1, -1, -1, -1,
          -1, 1, 1, 1}, {-1, -1, -1, -1, 1, -1, -1, -1}, {-1, -1, -1, -1, 1,
          -1, -1, 1}, {-1, -1, -1, -1, 1, -1, 1, -1}, {-1, -1, -1, -1, 1, -1,
          1, 1}, {-1, -1, -1, -1, 1, 1, -1, -1}, {-1, -1, -1, -1, 1, 1, -1, 1},
          {-1, -1, -1, -1, 1, 1, 1, -1}, {-1, -1, -1, -1, 1, 1, 1, 1}, {-1, -1,
          -1, 1, -1, -1, -1, -1}, {-1, -1, -1, 1, -1, -1, -1, 1}, {-1, -1, -1,
          1, -1, -1, 1, -1}, {-1, -1, -1, 1, -1, -1, 1, 1}, {-1, -1, -1, 1, -1,
          1, -1, -1}, {-1, -1, -1, 1, -1, 1, -1, 1}, {-1, -1, -1, 1, -1, 1, 1,
          -1}, {-1, -1, -1, 1, -1, 1, 1, 1}, {-1, -1, -1, 1, 1, -1, -1, -1},
          {-1, -1, -1, 1, 1, -1, -1, 1}, {-1, -1, -1, 1, 1, -1, 1, -1}, {-1,
          -1, -1, 1, 1, -1, 1, 1}, {-1, -1, -1, 1, 1, 1, -1, -1}, {-1, -1, -1,
          1, 1, 1, -1, 1}, {-1, -1, -1, 1, 1, 1, 1, -1}, {-1, -1, -1, 1, 1, 1,
          1, 1}, {-1, -1, 1, -1, -1, -1, -1, -1}, {-1, -1, 1, -1, -1, -1, -1,
          1}, {-1, -1, 1, -1, -1, -1, 1, -1}, {-1, -1, 1, -1, -1, -1, 1, 1},
          {-1, -1, 1, -1, -1, 1, -1, -1}, {-1, -1, 1, -1, -1, 1, -1, 1}, {-1,
          -1, 1, -1, -1, 1, 1, -1}, {-1, -1, 1, -1, -1, 1, 1, 1}, {-1, -1, 1,
          -1, 1, -1, -1, -1}, {-1, -1, 1, -1, 1, -1, -1, 1}, {-1, -1, 1, -1, 1,
          -1, 1, -1}, {-1, -1, 1, -1, 1, -1, 1, 1}, {-1, -1, 1, -1, 1, 1, -1,
          -1}, {-1, -1, 1, -1, 1, 1, -1, 1}, {-1, -1, 1, -1, 1, 1, 1, -1}, {-1,
          -1, 1, -1, 1, 1, 1, 1}, {-1, -1, 1, 1, -1, -1, -1, -1}, {-1, -1, 1,
          1, -1, -1, -1, 1}, {-1, -1, 1, 1, -1, -1, 1, -1}, {-1, -1, 1, 1, -1,
          -1, 1, 1}, {-1, -1, 1, 1, -1, 1, -1, -1}, {-1, -1, 1, 1, -1, 1, -1,
          1}, {-1, -1, 1, 1, -1, 1, 1, -1}, {-1, -1, 1, 1, -1, 1, 1, 1}, {-1,
          -1, 1, 1, 1, -1, -1, -1}, {-1, -1, 1, 1, 1, -1, -1, 1}, {-1, -1, 1,
          1, 1, -1, 1, -1}, {-1, -1, 1, 1, 1, -1, 1, 1}, {-1, -1, 1, 1, 1, 1,
          -1, -1}, {-1, -1, 1, 1, 1, 1, -1, 1}, {-1, -1, 1, 1, 1, 1, 1, -1},
          {-1, -1, 1, 1, 1, 1, 1, 1}, {-1, 1, -1, -1, -1, -1, -1, -1}, {-1, 1,
          -1, -1, -1, -1, -1, 1}, {-1, 1, -1, -1, -1, -1, 1, -1}, {-1, 1, -1,
          -1, -1, -1, 1, 1}, {-1, 1, -1, -1, -1, 1, -1, -1}, {-1, 1, -1, -1,
          -1, 1, -1, 1}, {-1, 1, -1, -1, -1, 1, 1, -1}, {-1, 1, -1, -1, -1, 1,
          1, 1}, {-1, 1, -1, -1, 1, -1, -1, -1}, {-1, 1, -1, -1, 1, -1, -1, 1},
          {-1, 1, -1, -1, 1, -1, 1, -1}, {-1, 1, -1, -1, 1, -1, 1, 1}, {-1, 1,
          -1, -1, 1, 1, -1, -1}, {-1, 1, -1, -1, 1, 1, -1, 1}, {-1, 1, -1, -1,
          1, 1, 1, -1}, {-1, 1, -1, -1, 1, 1, 1, 1}, {-1, 1, -1, 1, -1, -1, -1,
          -1}, {-1, 1, -1, 1, -1, -1, -1, 1}, {-1, 1, -1, 1, -1, -1, 1, -1},
          {-1, 1, -1, 1, -1, -1, 1, 1}, {-1, 1, -1, 1, -1, 1, -1, -1}, {-1, 1,
          -1, 1, -1, 1, -1, 1}, {-1, 1, -1, 1, -1, 1, 1, -1}, {-1, 1, -1, 1,
          -1, 1, 1, 1}, {-1, 1, -1, 1, 1, -1, -1, -1}, {-1, 1, -1, 1, 1, -1,
          -1, 1}, {-1, 1, -1, 1, 1, -1, 1, -1}, {-1, 1, -1, 1, 1, -1, 1, 1},
          {-1, 1, -1, 1, 1, 1, -1, -1}, {-1, 1, -1, 1, 1, 1, -1, 1}, {-1, 1,
          -1, 1, 1, 1, 1, -1}, {-1, 1, -1, 1, 1, 1, 1, 1}, {-1, 1, 1, -1, -1,
          -1, -1, -1}, {-1, 1, 1, -1, -1, -1, -1, 1}, {-1, 1, 1, -1, -1, -1, 1,
          -1}, {-1, 1, 1, -1, -1, -1, 1, 1}, {-1, 1, 1, -1, -1, 1, -1, -1},
          {-1, 1, 1, -1, -1, 1, -1, 1}, {-1, 1, 1, -1, -1, 1, 1, -1}, {-1, 1,
          1, -1, -1, 1, 1, 1}, {-1, 1, 1, -1, 1, -1, -1, -1}, {-1, 1, 1, -1, 1,
          -1, -1, 1}, {-1, 1, 1, -1, 1, -1, 1, -1}, {-1, 1, 1, -1, 1, -1, 1,
          1}, {-1, 1, 1, -1, 1, 1, -1, -1}, {-1, 1, 1, -1, 1, 1, -1, 1}, {-1,
          1, 1, -1, 1, 1, 1, -1}, {-1, 1, 1, -1, 1, 1, 1, 1}, {-1, 1, 1, 1, -1,
          -1, -1, -1}, {-1, 1, 1, 1, -1, -1, -1, 1}, {-1, 1, 1, 1, -1, -1, 1,
          -1}, {-1, 1, 1, 1, -1, -1, 1, 1}, {-1, 1, 1, 1, -1, 1, -1, -1}, {-1,
          1, 1, 1, -1, 1, -1, 1}, {-1, 1, 1, 1, -1, 1, 1, -1}, {-1, 1, 1, 1,
          -1, 1, 1, 1}, {-1, 1, 1, 1, 1, -1, -1, -1}, {-1, 1, 1, 1, 1, -1, -1,
          1}, {-1, 1, 1, 1, 1, -1, 1, -1}, {-1, 1, 1, 1, 1, -1, 1, 1}, {-1, 1,
          1, 1, 1, 1, -1, -1}, {-1, 1, 1, 1, 1, 1, -1, 1}, {-1, 1, 1, 1, 1, 1,
          1, -1}, {-1, 1, 1, 1, 1, 1, 1, 1}, {1, -1, -1, -1, -1, -1, -1, -1},
          {1, -1, -1, -1, -1, -1, -1, 1}, {1, -1, -1, -1, -1, -1, 1, -1}, {1,
          -1, -1, -1, -1, -1, 1, 1}, {1, -1, -1, -1, -1, 1, -1, -1}, {1, -1,
          -1, -1, -1, 1, -1, 1}, {1, -1, -1, -1, -1, 1, 1, -1}, {1, -1, -1, -1,
          -1, 1, 1, 1}, {1, -1, -1, -1, 1, -1, -1, -1}, {1, -1, -1, -1, 1, -1,
          -1, 1}, {1, -1, -1, -1, 1, -1, 1, -1}, {1, -1, -1, -1, 1, -1, 1, 1},
          {1, -1, -1, -1, 1, 1, -1, -1}, {1, -1, -1, -1, 1, 1, -1, 1}, {1, -1,
          -1, -1, 1, 1, 1, -1}, {1, -1, -1, -1, 1, 1, 1, 1}, {1, -1, -1, 1, -1,
          -1, -1, -1}, {1, -1, -1, 1, -1, -1, -1, 1}, {1, -1, -1, 1, -1, -1, 1,
          -1}, {1, -1, -1, 1, -1, -1, 1, 1}, {1, -1, -1, 1, -1, 1, -1, -1}, {1,
          -1, -1, 1, -1, 1, -1, 1}, {1, -1, -1, 1, -1, 1, 1, -1}, {1, -1, -1,
          1, -1, 1, 1, 1}, {1, -1, -1, 1, 1, -1, -1, -1}, {1, -1, -1, 1, 1, -1,
          -1, 1}, {1, -1, -1, 1, 1, -1, 1, -1}, {1, -1, -1, 1, 1, -1, 1, 1},
          {1, -1, -1, 1, 1, 1, -1, -1}, {1, -1, -1, 1, 1, 1, -1, 1}, {1, -1,
          -1, 1, 1, 1, 1, -1}, {1, -1, -1, 1, 1, 1, 1, 1}, {1, -1, 1, -1, -1,
          -1, -1, -1}, {1, -1, 1, -1, -1, -1, -1, 1}, {1, -1, 1, -1, -1, -1, 1,
          -1}, {1, -1, 1, -1, -1, -1, 1, 1}, {1, -1, 1, -1, -1, 1, -1, -1}, {1,
          -1, 1, -1, -1, 1, -1, 1}, {1, -1, 1, -1, -1, 1, 1, -1}, {1, -1, 1,
          -1, -1, 1, 1, 1}, {1, -1, 1, -1, 1, -1, -1, -1}, {1, -1, 1, -1, 1,
          -1, -1, 1}, {1, -1, 1, -1, 1, -1, 1, -1}, {1, -1, 1, -1, 1, -1, 1,
          1}, {1, -1, 1, -1, 1, 1, -1, -1}, {1, -1, 1, -1, 1, 1, -1, 1}, {1,
          -1, 1, -1, 1, 1, 1, -1}, {1, -1, 1, -1, 1, 1, 1, 1}, {1, -1, 1, 1,
          -1, -1, -1, -1}, {1, -1, 1, 1, -1, -1, -1, 1}, {1, -1, 1, 1, -1, -1,
          1, -1}, {1, -1, 1, 1, -1, -1, 1, 1}, {1, -1, 1, 1, -1, 1, -1, -1},
          {1, -1, 1, 1, -1, 1, -1, 1}, {1, -1, 1, 1, -1, 1, 1, -1}, {1, -1, 1,
          1, -1, 1, 1, 1}, {1, -1, 1, 1, 1, -1, -1, -1}, {1, -1, 1, 1, 1, -1,
          -1, 1}, {1, -1, 1, 1, 1, -1, 1, -1}, {1, -1, 1, 1, 1, -1, 1, 1}, {1,
          -1, 1, 1, 1, 1, -1, -1}, {1, -1, 1, 1, 1, 1, -1, 1}, {1, -1, 1, 1, 1,
          1, 1, -1}, {1, -1, 1, 1, 1, 1, 1, 1}, {1, 1, -1, -1, -1, -1, -1, -1},
          {1, 1, -1, -1, -1, -1, -1, 1}, {1, 1, -1, -1, -1, -1, 1, -1}, {1, 1,
          -1, -1, -1, -1, 1, 1}, {1, 1, -1, -1, -1, 1, -1, -1}, {1, 1, -1, -1,
          -1, 1, -1, 1}, {1, 1, -1, -1, -1, 1, 1, -1}, {1, 1, -1, -1, -1, 1, 1,
          1}, {1, 1, -1, -1, 1, -1, -1, -1}, {1, 1, -1, -1, 1, -1, -1, 1}, {1,
          1, -1, -1, 1, -1, 1, -1}, {1, 1, -1, -1, 1, -1, 1, 1}, {1, 1, -1, -1,
          1, 1, -1, -1}, {1, 1, -1, -1, 1, 1, -1, 1}, {1, 1, -1, -1, 1, 1, 1,
          -1}, {1, 1, -1, -1, 1, 1, 1, 1}, {1, 1, -1, 1, -1, -1, -1, -1}, {1,
          1, -1, 1, -1, -1, -1, 1}, {1, 1, -1, 1, -1, -1, 1, -1}, {1, 1, -1, 1,
          -1, -1, 1, 1}, {1, 1, -1, 1, -1, 1, -1, -1}, {1, 1, -1, 1, -1, 1, -1,
          1}, {1, 1, -1, 1, -1, 1, 1, -1}, {1, 1, -1, 1, -1, 1, 1, 1}, {1, 1,
          -1, 1, 1, -1, -1, -1}, {1, 1, -1, 1, 1, -1, -1, 1}, {1, 1, -1, 1, 1,
          -1, 1, -1}, {1, 1, -1, 1, 1, -1, 1, 1}, {1, 1, -1, 1, 1, 1, -1, -1},
          {1, 1, -1, 1, 1, 1, -1, 1}, {1, 1, -1, 1, 1, 1, 1, -1}, {1, 1, -1, 1,
          1, 1, 1, 1}, {1, 1, 1, -1, -1, -1, -1, -1}, {1, 1, 1, -1, -1, -1, -1,
          1}, {1, 1, 1, -1, -1, -1, 1, -1}, {1, 1, 1, -1, -1, -1, 1, 1}, {1, 1,
          1, -1, -1, 1, -1, -1}, {1, 1, 1, -1, -1, 1, -1, 1}, {1, 1, 1, -1, -1,
          1, 1, -1}, {1, 1, 1, -1, -1, 1, 1, 1}, {1, 1, 1, -1, 1, -1, -1, -1},
          {1, 1, 1, -1, 1, -1, -1, 1}, {1, 1, 1, -1, 1, -1, 1, -1}, {1, 1, 1,
          -1, 1, -1, 1, 1}, {1, 1, 1, -1, 1, 1, -1, -1}, {1, 1, 1, -1, 1, 1,
          -1, 1}, {1, 1, 1, -1, 1, 1, 1, -1}, {1, 1, 1, -1, 1, 1, 1, 1}, {1, 1,
          1, 1, -1, -1, -1, -1}, {1, 1, 1, 1, -1, -1, -1, 1}, {1, 1, 1, 1, -1,
          -1, 1, -1}, {1, 1, 1, 1, -1, -1, 1, 1}, {1, 1, 1, 1, -1, 1, -1, -1},
          {1, 1, 1, 1, -1, 1, -1, 1}, {1, 1, 1, 1, -1, 1, 1, -1}, {1, 1, 1, 1,
          -1, 1, 1, 1}, {1, 1, 1, 1, 1, -1, -1, -1}, {1, 1, 1, 1, 1, -1, -1,
          1}, {1, 1, 1, 1, 1, -1, 1, -1}, {1, 1, 1, 1, 1, -1, 1, 1}, {1, 1, 1,
          1, 1, 1, -1, -1}, {1, 1, 1, 1, 1, 1, -1, 1}, {1, 1, 1, 1, 1, 1, 1,
          -1}, {1, 1, 1, 1, 1, 1, 1, 1}};

      // Private functions to calculate the matrix element for all subprocesses
      // Calculate wavefunctions
      void calculate_wavefunctions(const int perm[], const int hel[]); 
      std::complex<double> amp[4]; 
      double matrix_1_gg_ttx_t_wpb_wp_mupvm_tx_wmbx_wm_mumvmx(); 
      double matrix_1_uux_ttx_t_wpb_wp_mupvm_tx_wmbx_wm_mumvmx(); 
      double matrix_1_gg_ttx_t_wpb_wp_epve_tx_wmbx_wm_mumvmx(); 
      double matrix_1_uux_ttx_t_wpb_wp_epve_tx_wmbx_wm_mumvmx(); 
      double matrix_1_gg_ttx_t_wpb_wp_mupvm_tx_wmbx_wm_emvex(); 
      double matrix_1_uux_ttx_t_wpb_wp_mupvm_tx_wmbx_wm_emvex(); 
      double matrix_1_gg_ttx_t_wpb_wp_epve_tx_wmbx_wm_emvex(); 
      double matrix_1_uux_ttx_t_wpb_wp_epve_tx_wmbx_wm_emvex(); 

      // map of final states
      std::map < std::vector<int> , std::vector < __MatrixElement <
          cpp_pp_ttx_fullylept >> > mapFinalStates;

      // Reference to the model parameters instance passed in the constructor
      Parameters_sm& params; 

      // vector with external particle masses
      std::vector<double> mME; 

      // vector with momenta (to be changed each event)
      double * momenta[8]; 
  }; 



  #endif  // MG5_Sigma_sm_gg_mupvmbmumvmxbx_H

