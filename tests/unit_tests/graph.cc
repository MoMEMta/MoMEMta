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

/**
 * \file
 * \brief Unit tests for graph
 * \ingroup UnitTests
 */

#include <catch.hpp>

#include <momemta/ConfigurationReader.h>
#include <momemta/Logging.h>

#include <Graph.h>

Configuration get_conf(const std::string& conf) {
    auto reader = ConfigurationReader("!" + conf);
    return reader.freeze();
}

TEST_CASE("Graph", "[core][graph]") {

    logging::set_level(logging::level::warning);

    momemta::ModuleList available_modules;
    momemta::ModuleRegistry::get().exportList(false, available_modules);

    SECTION("Single module") {
        const std::string conf_str = R"(
local input = declare_input("input")

DoubleConstant.constant = { value = 42. }

integrand("constant::value")
)";

        auto conf = get_conf(conf_str);

        REQUIRE(conf.getIntegrands().size() == 1);
        REQUIRE(conf.getInputs().size() == 1);
        REQUIRE(conf.getModules().size() == 5); // 1 user + 4 internals

        momemta::ComputationGraphBuilder builder(available_modules, conf);
        auto graph = builder.build();

        REQUIRE(graph->getPaths().size() == 1);
        REQUIRE(graph->getPaths().front() == DEFAULT_EXECUTION_PATH);

        auto modules = graph->getDecls(DEFAULT_EXECUTION_PATH);
        REQUIRE(modules.size() == 1);
        REQUIRE(modules.front().type == "DoubleConstant");
        REQUIRE(modules.front().name == "constant");
    }

    SECTION("Unused module should be removed") {
        const std::string conf_str = R"(
local input = declare_input("input")

DoubleConstant.constant = { value = 42. }

DoubleConstant.unused = { value = 42. }

integrand("constant::value")
)";

        auto conf = get_conf(conf_str);

        REQUIRE(conf.getModules().size() == 6); // 2 user + 4 internals

        momemta::ComputationGraphBuilder builder(available_modules, conf);
        auto graph = builder.build();

        REQUIRE(graph->getPaths().size() == 1);
        REQUIRE(graph->getPaths().front() == DEFAULT_EXECUTION_PATH);

        auto modules = graph->getDecls(DEFAULT_EXECUTION_PATH);
        REQUIRE(modules.size() == 1);
    }

    SECTION("Sticky module should not be removed") {
        const std::string conf_str = R"(
local input = declare_input("input")

DoubleConstant.constant = { value = 42. }

DoublePrinter.unused = { input = "constant::value" }

integrand("constant::value")
)";

        auto conf = get_conf(conf_str);

        REQUIRE(conf.getModules().size() == 6); // 2 user + 4 internals

        momemta::ComputationGraphBuilder builder(available_modules, conf);
        auto graph = builder.build();

        REQUIRE(graph->getPaths().size() == 1);
        REQUIRE(graph->getPaths().front() == DEFAULT_EXECUTION_PATH);

        auto modules = graph->getDecls(DEFAULT_EXECUTION_PATH);
        REQUIRE(modules.size() == 2);
    }

    SECTION("Module should be correctly ordered") {
        const std::string conf_str = R"(
local input_a = declare_input("input_a")
local input_b = declare_input("input_b")

GaussianTransferFunctionOnEnergy.tf = {
    ps_point = add_dimension(),
    reco_particle = "input_sum::output",
    sigma = 0.05
}

VectorLinearCombinator.input_sum = {
    inputs = { input_a.reco_p4, input_b.reco_p4 },
    coefficients = {1, 1}
}

integrand("tf::output")
)";

        auto conf = get_conf(conf_str);

        REQUIRE(conf.getModules().size() == 7); // 2 user + 5 internals

        momemta::ComputationGraphBuilder builder(available_modules, conf);
        auto graph = builder.build();

        REQUIRE(graph->getPaths().size() == 1);
        REQUIRE(graph->getPaths().front() == DEFAULT_EXECUTION_PATH);

        auto modules = graph->getDecls(DEFAULT_EXECUTION_PATH);
        REQUIRE(modules.size() == 2);

        REQUIRE(modules.at(0).name == "input_sum");
        REQUIRE(modules.at(1).name == "tf");
    }

    SECTION("Circular dependencies are not allowed") {
        const std::string conf_str = R"(
GaussianTransferFunctionOnEnergy.tf_1 = {
    ps_point = "dummy::dummy",
    reco_particle = "tf_2::output",
    sigma = 0.05
}

GaussianTransferFunctionOnEnergy.tf_2 = {
    ps_point = "dummy::dummy",
    reco_particle = "tf_1::output",
    sigma = 0.05
}
)";

        auto conf = get_conf(conf_str);

        REQUIRE(conf.getModules().size() == 5); // 2 user + 3 internals

        logging::set_level(logging::level::off);
        momemta::ComputationGraphBuilder builder(available_modules, conf);
        REQUIRE_THROWS(builder.build());
    }

    SECTION("Two execution paths") {
        const std::string conf_str = R"(
Looper.looper = {
    solutions = "dummy::dummy",
    path = Path("sum", "constant_1", "constant_2", "printer")
}

SolutionPrinter.printer = { input = "looper::particles" }
DoubleConstant.constant_1 = { value = 42. }
DoubleConstant.constant_2 = { value = 42. }

DoubleLinearCombinator.sum = {
    inputs = { "constant_1::value", "constant_2::value" },
    coefficients = {1, 1}
}

integrand("sum::output")
)";

        auto conf = get_conf(conf_str);

        REQUIRE(conf.getModules().size() == 8); // 5 user + 3 internals
        REQUIRE(conf.getPaths().size() == 1);

        momemta::ComputationGraphBuilder builder(available_modules, conf);
        auto graph = builder.build();

        REQUIRE(graph->getPaths().size() == 2);
        REQUIRE(graph->getPaths().front() == DEFAULT_EXECUTION_PATH);
        REQUIRE(graph->getPaths().back() == conf.getPaths().front()->id);

        auto modules = graph->getDecls(DEFAULT_EXECUTION_PATH);
        REQUIRE(modules.size() == 1); // Only the looper

        REQUIRE(modules.at(0).name == "looper");

        modules = graph->getDecls(graph->getPaths().back());
        // Order is not guaranteed, only test the number of modules
        REQUIRE(modules.size() == 4);
    }

    SECTION("A module using looper's output must be inside the looper execution path") {
        const std::string conf_str = R"(
Looper.looper = {
    solutions = "dummy::dummy",
    path = Path("constant")
}

SolutionPrinter.printer = { input = "looper::particles" }
DoubleConstant.constant = { value = 42. }

integrand("sum::output")
)";

        auto conf = get_conf(conf_str);

        REQUIRE(conf.getModules().size() == 6); // 3 user + 3 internals
        REQUIRE(conf.getPaths().size() == 1);

        logging::set_level(logging::level::off);
        momemta::ComputationGraphBuilder builder(available_modules, conf);
        REQUIRE_THROWS(
                builder.build()
        );
    }

    SECTION("All module inside the execution path must exists") {
        const std::string conf_str = R"(
Looper.looper = {
    solutions = "dummy::dummy",
    path = Path("unexisting", "printer")
}

SolutionPrinter.printer = { input = "looper::particles" }

integrand("dummy::dummy")
)";

        auto conf = get_conf(conf_str);

        REQUIRE(conf.getModules().size() == 5); // 2 user + 3 internals
        REQUIRE(conf.getPaths().size() == 1);

        logging::set_level(logging::level::off);
        momemta::ComputationGraphBuilder builder(available_modules, conf);
        auto graph = builder.build();
    }

    SECTION("Three execution paths") {
        const std::string conf_str = R"(

Looper.looper_2 = {
    solutions = "dummy_2::dummy",
    path = Path("printer_2")
}

Looper.looper_1 = {
    solutions = "dummy_1::dummy",
    path = Path("printer_1", "looper_2")
}

SolutionPrinter.printer_1 = { input = "looper_1::particles" }
SolutionPrinter.printer_2 = { input = "looper_2::particles" }

integrand("some::output")
)";

        auto conf = get_conf(conf_str);

        REQUIRE(conf.getModules().size() == 7); // 4 user + 3 internals
        REQUIRE(conf.getPaths().size() == 2);

        momemta::ComputationGraphBuilder builder(available_modules, conf);
        auto graph = builder.build();

        REQUIRE(graph->getPaths().size() == 3);
        REQUIRE(graph->getPaths().front() == DEFAULT_EXECUTION_PATH);

        // Last execution path should the one from looper_2, declared first in the configuration
        REQUIRE(graph->getPaths().back() == conf.getPaths().front()->id);

        auto modules = graph->getDecls(DEFAULT_EXECUTION_PATH);
        REQUIRE(modules.size() == 1); // Only looper_1

        REQUIRE(modules.at(0).name == "looper_1");

        modules = graph->getDecls(graph->getPaths().at(1));

        // Looper_1 path, printer_1 and looper_2
        REQUIRE(modules.size() == 2);

        modules = graph->getDecls(graph->getPaths().at(2));

        REQUIRE(modules.size() == 1);
        REQUIRE(modules.at(0).name == "printer_2");
    }

    SECTION("Unused module should not increase the number of dimension") {
        const std::string conf_str = R"(

local first_dim = add_dimension()
local unused_dim = add_dimension()

GaussianTransferFunctionOnEnergy.tf_1 = {
    ps_point = first_dim,
    reco_particle = "dummy::output",
    sigma = 0.05
}

GaussianTransferFunctionOnEnergy.tf_2 = {
    ps_point = first_dim,
    reco_particle = "dummy::output",
    sigma = 0.05
}

GaussianTransferFunctionOnEnergy.tf_3 = {
    ps_point = add_dimension(),
    reco_particle = "dummy::output",
    sigma = 0.05
}

GaussianTransferFunctionOnEnergy.unused = {
    ps_point = unused_dim,
    reco_particle = "dummy::output",
    sigma = 0.05
}

integrand("tf_1::output", "tf_2::output", "tf_3::output")
)";

        auto conf = get_conf(conf_str);

        // Just parsing the configuration file leads to 3 dimensions, with one requested by a unused module
        REQUIRE(conf.getNDimensions() == 3);

        momemta::ComputationGraphBuilder builder(available_modules, conf);
        auto graph = builder.build();

        // The dimension requested by the unused module should be discarded
        REQUIRE(graph->getNDimensions() == 2);

        REQUIRE(graph->getPaths().size() == 1);
        REQUIRE(graph->getPaths().front() == DEFAULT_EXECUTION_PATH);

        auto modules = graph->getDecls(DEFAULT_EXECUTION_PATH);
        REQUIRE(modules.size() == 3);
    }
}