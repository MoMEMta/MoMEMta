/*
 *  MoMEMta: a modular implementation of the Matrix Element Method
 *  Copyright (C) 2016  Universite catholique de Louvain (UCL), Belgium
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

#include <momemta/Configuration.h>
#include <momemta/ConfigurationReader.h>
#include <momemta/MoMEMta.h>
#include <momemta/Logging.h>

#include <boost/python.hpp>

#ifdef ROOT_HAS_PYROOT
#include <TPython.h>
#endif

/**
 * \file
 * \brief Python bindings
 * \defgroup Python Python bindings
 */

namespace bp = boost::python;

void set_log_level(boost::log::trivial::severity_level severity) {
    logging::set_level(severity);
}

bp::list MoMEMta_computeWeights_MET(MoMEMta& m, bp::list particles_, bp::list met_) {
    std::vector<LorentzVector> particles;
    for (ssize_t i = 0; i < bp::len(particles_); i++) {
        particles.push_back(bp::extract<LorentzVector>(particles_[i]));
    }

    LorentzVector met;
    bp::extract<LorentzVector> lorentzVectorExtractor(met_);
    if (lorentzVectorExtractor.check())
        met = lorentzVectorExtractor();

    auto weights = m.computeWeights(particles, met);

    bp::list result;
    for (const auto& weight: weights) {
        bp::tuple pair = bp::make_tuple(weight.first, weight.second);
        result.append(pair);
    }

    return result;
}

bp::list MoMEMta_computeWeights(MoMEMta& m, bp::list particles) {
    return MoMEMta_computeWeights_MET(m, particles, bp::list());
}

template<typename T>
const T& ParameterSet_get(ParameterSet& p, const std::string& name) {
    return p.get<T>(name);
}

template<typename T>
void ParameterSet_set(ParameterSet& p, const std::string& name, const T& value) {
    return p.set<T>(name, value);
}

struct LorentzVector_to_python {
    static PyObject* convert(LorentzVector const& s) {
        bp::list result;
        result.append(s.X());
        result.append(s.Y());
        result.append(s.Z());
        result.append(s.T());

        return boost::python::incref(result.ptr());
    }
};

struct LorentzVector_from_python {
    LorentzVector_from_python() {
        bp::converter::registry::push_back(
                &convertible,
                &construct,
                boost::python::type_id<LorentzVector>());
    }

    static void* convertible(PyObject* obj_ptr) {
        if (! PyList_Check(obj_ptr))
            return nullptr;

        if (PyList_Size(obj_ptr) != 4)
            return nullptr;

        return obj_ptr;
    }

    static void construct(
            PyObject* obj_ptr,
            bp::converter::rvalue_from_python_stage1_data* data) {
        //if (value == 0) boost::python::throw_error_already_set();

        void* storage = (
                (bp::converter::rvalue_from_python_storage<LorentzVector>*)
                        data)->storage.bytes;
        new (storage) LorentzVector(
                bp::extract<double>(PyList_GET_ITEM(obj_ptr, 0)),
                bp::extract<double>(PyList_GET_ITEM(obj_ptr, 1)),
                bp::extract<double>(PyList_GET_ITEM(obj_ptr, 2)),
                bp::extract<double>(PyList_GET_ITEM(obj_ptr, 3))
        );
        data->convertible = storage;
    }
};

#ifdef ROOT_HAS_PYROOT
template <typename T>
struct convert_py_root_to_cpp_root {
    convert_py_root_to_cpp_root() {
        bp::converter::registry::push_back(&convertible, &construct,
                                           bp::type_id<T>());
    }
    static void* convertible(PyObject* obj_ptr) {
        return TPython::ObjectProxy_Check(obj_ptr) ? obj_ptr : nullptr;
    }

    static void construct(PyObject* obj_ptr,
                          bp::converter::rvalue_from_python_stage1_data* data) {
        T* TObj = static_cast<T*>(TPython::ObjectProxy_AsVoidPtr(obj_ptr));
        data->convertible = TObj;
    }
};
#endif

// Overloads for MoMEMta::computeWeights
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(MoMEMta_computeWeights_overloads, MoMEMta::computeWeights, 1, 2)

BOOST_PYTHON_MODULE(momemta) {

    using namespace boost::python;

    to_python_converter<LorentzVector, LorentzVector_to_python>();
    LorentzVector_from_python();

#ifdef ROOT_HAS_PYROOT
    convert_py_root_to_cpp_root<LorentzVector>();
#endif

    enum_<boost::log::trivial::severity_level>("log_level")
            .value("trace", boost::log::trivial::severity_level::trace)
            .value("debug", boost::log::trivial::severity_level::debug)
            .value("info", boost::log::trivial::severity_level::info)
            .value("warning", boost::log::trivial::severity_level::warning)
            .value("error", boost::log::trivial::severity_level::error)
            .value("fatal", boost::log::trivial::severity_level::fatal);

    def("set_log_level", set_log_level);

    class_<ParameterSet>("ParameterSet", no_init)
            .def("exists", &ParameterSet::exists)
            .def("getDouble", ParameterSet_get<double>, return_value_policy<copy_const_reference>())
            .def("getInt", ParameterSet_get<int>, return_value_policy<copy_const_reference>())
            .def("getString", ParameterSet_get<std::string>, return_value_policy<copy_const_reference>())
            .def("getInputTag", ParameterSet_get<InputTag>, return_value_policy<copy_const_reference>())
            .def("getParameterSet", ParameterSet_get<ParameterSet>, return_value_policy<copy_const_reference>())
            .def("setDouble", ParameterSet_set<double>)
            .def("setInt", ParameterSet_set<int>)
            .def("setString", ParameterSet_set<std::string>)
            .def("setInputTag", ParameterSet_set<InputTag>);

    class_<Configuration>("Configuration", no_init)
            .def("getGlobalParameters", &Configuration::getGlobalParameters,
                 return_internal_reference<>())
            .def("getCubaConfiguration", &Configuration::getCubaConfiguration,
                 return_internal_reference<>());

    class_<ConfigurationReader>("ConfigurationReader", init<std::string>())
            .def("freeze", &ConfigurationReader::freeze)
            .def("getGlobalParameters", &ConfigurationReader::getGlobalParameters,
                 return_value_policy<reference_existing_object>())
            .def("getCubaConfiguration", &ConfigurationReader::getCubaConfiguration,
                 return_value_policy<reference_existing_object>());

    enum_<MoMEMta::IntegrationStatus>("IntegrationStatus")
            .value("ABORTED", MoMEMta::IntegrationStatus::ABORTED)
            .value("ACCURACY_NOT_REACHED", MoMEMta::IntegrationStatus::ACCURARY_NOT_REACHED)
            .value("DIM_OUT_OF_RANGE", MoMEMta::IntegrationStatus::DIM_OUT_OF_RANGE)
            .value("FAILED", MoMEMta::IntegrationStatus::FAILED)
            .value("NONE", MoMEMta::IntegrationStatus::NONE)
            .value("SUCCESS", MoMEMta::IntegrationStatus::SUCCESS);

    class_<MoMEMta>("MoMEMta", init<Configuration>())
            .def("getIntegrationStatus", &MoMEMta::getIntegrationStatus)
            //.def("getPool", &MoMEMta::getPool, return_value_policy<copy_const_reference>())
            .def("computeWeights", MoMEMta_computeWeights)
            .def("computeWeights", MoMEMta_computeWeights_MET)
            .def("computeWeights", &MoMEMta::computeWeights, MoMEMta_computeWeights_overloads());
}