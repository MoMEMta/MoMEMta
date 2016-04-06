#include <momemta/MatrixElement.h>
#include <momemta/Utils.h>

class DummyMatrixElement : public momemta::MatrixElement {
  public:
    // Constructor & destructor
    DummyMatrixElement(const ConfigurationSet& configuration) {
        UNUSED(configuration);
    }

    virtual ~DummyMatrixElement(){};

    // Calculate flavour-independent parts of cross section.
    virtual momemta::MatrixElement::Result
    compute(const std::vector<std::vector<double>>& initialMomenta,
            const std::vector<std::pair<int, std::vector<double>>>& finalState) {
        UNUSED(initialMomenta);
        UNUSED(finalState);
        return {};
    }

    virtual std::shared_ptr<momemta::MEParameters> getParameters() {
        return std::shared_ptr<momemta::MEParameters>();
    }
};

#include <momemta/MatrixElementFactory.h>
REGISTER_MATRIX_ELEMENT("dummy_matrix_element", DummyMatrixElement);
