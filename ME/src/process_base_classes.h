#ifndef BASE_CLASSES_H
#define BASE_CLASSES_H

#include <vector> 
#include <utility>
#include <map>

class CPPProcess
{
  public:

  virtual std::map< std::pair<int, int> , double > sigmaKin(
      const std::vector< std::vector<double> > &initialMomenta, 
	  const std::vector< std::pair< int, std::vector<double> > > &finalState
	  ) = 0;
  virtual ~CPPProcess(){};
};
  

template<typename TClass> class __MatrixElement
{
  public:

    __MatrixElement(double(TClass::*myMeCall)(), bool mirror,
        std::vector< std::pair<int, int> > iniStates, int ncomb, int
        denom):
    meCall(myMeCall), 
    hasMirrorProcess(mirror), 
    initialStates(iniStates), 
    goodHel(ncomb, true), 
    denominator(denom)
    {}

    double(TClass::*meCall)(); 
    bool hasMirrorProcess; 
    std::vector< std::pair<int, int> > initialStates; 
    std::vector<bool> goodHel; 
    int denominator;

  private:

	// Hide default constructor
    __MatrixElement(); 

}; 

#endif // BASE_CLASSES_H

