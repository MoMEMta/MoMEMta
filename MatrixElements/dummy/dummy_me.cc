#include <momemta/MatrixElement.h>
#include <momemta/Unused.h>

class DummyMatrixElement : public momemta::MatrixElement {
  public:
    // Constructor & destructor
    DummyMatrixElement(const ParameterSet& configuration) {
        UNUSED(configuration);
    }

    virtual ~DummyMatrixElement(){};

    virtual void resetHelicities(){};

    // Calculate flavour-independent parts of cross section.
    virtual momemta::MatrixElement::Result
    compute(const std::pair<std::vector<double>, std::vector<double>>& initialMomenta,
            const std::vector<std::pair<int, std::vector<double>>>& finalState) {
        UNUSED(initialMomenta);
        UNUSED(finalState);
        
        // Initialise dummy result object
        std::map < std::pair < int, int > , double > result;
        result[std::make_pair(0,0)] = 1;

        return result;
    }

    virtual std::shared_ptr<momemta::MEParameters> getParameters() {
        return std::shared_ptr<momemta::MEParameters>();
    }
};

#include <momemta/MatrixElementFactory.h>
REGISTER_MATRIX_ELEMENT("dummy_matrix_element", DummyMatrixElement);
