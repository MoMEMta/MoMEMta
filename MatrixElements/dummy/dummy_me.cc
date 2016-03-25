#include <momemta/MatrixElement.h>

class DummyMatrixElement : public momemta::MatrixElement {
  public:
    // Constructor & destructor
    DummyMatrixElement(const ConfigurationSet& configuration) {}
    virtual ~DummyMatrixElement(){};

    // Calculate flavour-independent parts of cross section.
    virtual momemta::MatrixElement::Result
    compute(const std::vector<std::vector<double>>& initialMomenta,
            const std::vector<std::pair<int, std::vector<double>>>& finalState) {
        return {};
    }

    virtual std::shared_ptr<momemta::MEParameters> getParameters() {
        return std::shared_ptr<momemta::MEParameters>();
    }

    // Info on the subprocess.
    virtual std::string name() const { return "Dummy matrix element"; }

    virtual const std::vector<double>& getMasses() const {
        static std::vector<double> s_vector;
        return s_vector;
    }
};

#include <momemta/MatrixElementFactory.h>
REGISTER_MATRIX_ELEMENT("dummy_matrix_element", DummyMatrixElement);
