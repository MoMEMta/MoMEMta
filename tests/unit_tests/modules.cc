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

/**
 * \file
 * \brief Unit tests for modules
 * \sa Module
 * \ingroup UnitTests
 */

#include <catch.hpp>

#include <momemta/ModuleFactory.h>
#include <momemta/Module.h>
#include <momemta/ParameterSet.h>
#include <momemta/Pool.h>

#define N_PS_POINTS 5

// A mock of ParameterSet to change visibility of the constructor
class ParameterSetMock: public ParameterSet {
public:
    ParameterSetMock(const std::string& name):
            ParameterSet(name, name) {
        // Empty
    }
};

std::shared_ptr<std::vector<double>> addPhaseSpacePoints(std::shared_ptr<Pool> pool) {
    pool->current_module("cuba");

    auto ps_points = pool->put<std::vector<double>>({"cuba", "ps_points"});
    auto weight = pool->put<double>({"cuba", "ps_weight"});

    double step = 1. / (N_PS_POINTS - 1);

    ps_points->resize(N_PS_POINTS);
    for (size_t i = 0; i < N_PS_POINTS; i++) {
        double point = i * step;
        ps_points->operator[](i) = point;
    }

    *weight = 1;

    return ps_points;
}

TEST_CASE("Modules", "[modules]") {
    std::shared_ptr<Pool> pool(new Pool());
    std::shared_ptr<ParameterSetMock> parameters;

    auto createModule = [&pool, &parameters](const std::string& type) {
        Configuration::Module module;
        module.name = type;
        module.type = type;
        module.parameters.reset(parameters->clone());

        pool->current_module(module);

        auto result = ModuleFactory::get().create(type, pool, *parameters);
        REQUIRE(result.get());

        return result;
    };

    // Register phase space points, mocking what cuba would do
    auto ps_points = addPhaseSpacePoints(pool);

    SECTION("BreitWignerGenerator") {

        parameters.reset(new ParameterSetMock("BreitWignerGenerator"));

        double mass = 173.;
        double width = 5.;

        parameters->set("mass", mass);
        parameters->set("width", width);
        parameters->set("ps_point", InputTag("cuba", "ps_points", 2)); // 0.5 by default

        auto module = createModule("BreitWignerGenerator");

        auto s = pool->get<double>({"BreitWignerGenerator", "s"});
        auto jacobian = pool->get<double>({"BreitWignerGenerator", "jacobian"});

        REQUIRE(module->work() == Module::Status::OK);

        REQUIRE(*s == Approx(29941.5));
        REQUIRE(*jacobian == Approx(2693.05));

        ps_points->operator[](2) = 0;

        REQUIRE(module->work() == Module::Status::OK);

        REQUIRE(*s == Approx(0));
    }

    SECTION("UniformGenerator") {

        parameters.reset(new ParameterSetMock("UniformGenerator"));

        double min = 100;
        double max = 250;
        double expected_jacobian = max - min;

        parameters->set("min", min);
        parameters->set("max", max);
        parameters->set("ps_point", InputTag("cuba", "ps_points", 0));

        auto module = createModule("UniformGenerator");

        auto output = pool->get<double>({"UniformGenerator", "output"});
        auto jacobian = pool->get<double>({"UniformGenerator", "jacobian"});

        ps_points->operator[](0) = 0;

        REQUIRE(module->work() == Module::Status::OK);

        REQUIRE(*output == Approx(min));
        REQUIRE(*jacobian == Approx(expected_jacobian));

        ps_points->operator[](0) = 1;

        REQUIRE(module->work() == Module::Status::OK);

        REQUIRE(*output == Approx(max));
        REQUIRE(*jacobian == Approx(expected_jacobian));

        ps_points->operator[](0) = 0.5;

        REQUIRE(module->work() == Module::Status::OK);

        REQUIRE(*output == Approx((max + min) / 2.));
        REQUIRE(*jacobian == Approx(expected_jacobian));
    }
}