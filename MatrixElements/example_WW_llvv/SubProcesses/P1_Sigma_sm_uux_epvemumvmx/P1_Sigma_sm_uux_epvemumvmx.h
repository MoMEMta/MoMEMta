#pragma once

#include <complex> 
#include <vector> 
#include <utility> 
#include <map> 

#include <Parameters_sm.h> 
#include <SubProcess.h> 

#include <momemta/MatrixElement.h> 

namespace example_WW_llvv 
{

//==========================================================================
// A class for calculating the matrix elements for
// Process: u u~ > w+ w- WEIGHTED<=4 @1
// *   Decay: w+ > e+ ve WEIGHTED<=2
// *   Decay: w- > mu- vm~ WEIGHTED<=2
// Process: c c~ > w+ w- WEIGHTED<=4 @1
// *   Decay: w+ > e+ ve WEIGHTED<=2
// *   Decay: w- > mu- vm~ WEIGHTED<=2
// Process: d d~ > w+ w- WEIGHTED<=4 @1
// *   Decay: w+ > e+ ve WEIGHTED<=2
// *   Decay: w- > mu- vm~ WEIGHTED<=2
// Process: s s~ > w+ w- WEIGHTED<=4 @1
// *   Decay: w+ > e+ ve WEIGHTED<=2
// *   Decay: w- > mu- vm~ WEIGHTED<=2
//--------------------------------------------------------------------------

class P1_Sigma_sm_uux_epvemumvmx: public momemta::MatrixElement 
{
  public:

    // Constructor & destructor
    P1_Sigma_sm_uux_epvemumvmx(const ParameterSet& configuration); 
    virtual ~P1_Sigma_sm_uux_epvemumvmx() {}; 

    // Calculate flavour-independent parts of cross section.
    virtual momemta::MatrixElement::Result compute(
    const std::pair < std::vector<double> , std::vector<double> >
        &initialMomenta,
    const std::vector < std::pair < int, std::vector<double> > > &finalState); 

    virtual std::shared_ptr < momemta::MEParameters > getParameters() 
    {
      return params; 
    }

    // needed? const std::vector<double>& getMasses() const {return mME;}

  private:

    // default constructor should be hidden
    P1_Sigma_sm_uux_epvemumvmx() = delete; 

    // list of helicities combinations
    const int helicities[64][6] = {{-1, -1, -1, -1, -1, -1}, {-1, -1, -1, -1,
        -1, 1}, {-1, -1, -1, -1, 1, -1}, {-1, -1, -1, -1, 1, 1}, {-1, -1, -1,
        1, -1, -1}, {-1, -1, -1, 1, -1, 1}, {-1, -1, -1, 1, 1, -1}, {-1, -1,
        -1, 1, 1, 1}, {-1, -1, 1, -1, -1, -1}, {-1, -1, 1, -1, -1, 1}, {-1, -1,
        1, -1, 1, -1}, {-1, -1, 1, -1, 1, 1}, {-1, -1, 1, 1, -1, -1}, {-1, -1,
        1, 1, -1, 1}, {-1, -1, 1, 1, 1, -1}, {-1, -1, 1, 1, 1, 1}, {-1, 1, -1,
        -1, -1, -1}, {-1, 1, -1, -1, -1, 1}, {-1, 1, -1, -1, 1, -1}, {-1, 1,
        -1, -1, 1, 1}, {-1, 1, -1, 1, -1, -1}, {-1, 1, -1, 1, -1, 1}, {-1, 1,
        -1, 1, 1, -1}, {-1, 1, -1, 1, 1, 1}, {-1, 1, 1, -1, -1, -1}, {-1, 1, 1,
        -1, -1, 1}, {-1, 1, 1, -1, 1, -1}, {-1, 1, 1, -1, 1, 1}, {-1, 1, 1, 1,
        -1, -1}, {-1, 1, 1, 1, -1, 1}, {-1, 1, 1, 1, 1, -1}, {-1, 1, 1, 1, 1,
        1}, {1, -1, -1, -1, -1, -1}, {1, -1, -1, -1, -1, 1}, {1, -1, -1, -1, 1,
        -1}, {1, -1, -1, -1, 1, 1}, {1, -1, -1, 1, -1, -1}, {1, -1, -1, 1, -1,
        1}, {1, -1, -1, 1, 1, -1}, {1, -1, -1, 1, 1, 1}, {1, -1, 1, -1, -1,
        -1}, {1, -1, 1, -1, -1, 1}, {1, -1, 1, -1, 1, -1}, {1, -1, 1, -1, 1,
        1}, {1, -1, 1, 1, -1, -1}, {1, -1, 1, 1, -1, 1}, {1, -1, 1, 1, 1, -1},
        {1, -1, 1, 1, 1, 1}, {1, 1, -1, -1, -1, -1}, {1, 1, -1, -1, -1, 1}, {1,
        1, -1, -1, 1, -1}, {1, 1, -1, -1, 1, 1}, {1, 1, -1, 1, -1, -1}, {1, 1,
        -1, 1, -1, 1}, {1, 1, -1, 1, 1, -1}, {1, 1, -1, 1, 1, 1}, {1, 1, 1, -1,
        -1, -1}, {1, 1, 1, -1, -1, 1}, {1, 1, 1, -1, 1, -1}, {1, 1, 1, -1, 1,
        1}, {1, 1, 1, 1, -1, -1}, {1, 1, 1, 1, -1, 1}, {1, 1, 1, 1, 1, -1}, {1,
        1, 1, 1, 1, 1}};

    // Private functions to calculate the matrix element for all subprocesses
    // Wavefunctions
    void calculate_wavefunctions(const int perm[], const int hel[]); 
    std::complex<double> amp[6]; 

    // Matrix elements
    double matrix_1_uux_wpwm_wp_epve_wm_mumvmx(); 
    double matrix_1_ddx_wpwm_wp_epve_wm_mumvmx(); 

    // map of final states
    std::map < std::vector<int> , std::vector < Subprocess <
        cpp_pp_ttx_fullylept >> > mapFinalStates;

    // Reference to the model parameters instance passed in the constructor
    std::shared_ptr < Parameters_sm > params; 

    // vector with external particle masses
    std::vector<double> mME; 

    // vector with momenta (to be changed each event)
    double * momenta[6]; 
}; 


}

