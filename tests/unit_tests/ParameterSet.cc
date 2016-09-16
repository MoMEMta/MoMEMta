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
 * \brief Unit tests for ParameterSet
 * \ingroup UnitTests
 */

#include <catch.hpp>

#include <momemta/InputTag.h>
#include <momemta/ParameterSet.h>

TEST_CASE("ParameterSet unit tests", "[core]") {
    ParameterSet p;

    SECTION("Adding bool") {
        REQUIRE_FALSE(p.existsAs<bool>("parameter"));

        p.set("parameter", false);

        REQUIRE(p.existsAs<bool>("parameter"));
        REQUIRE_FALSE(p.get<bool>("parameter"));
    }

    SECTION("Adding string") {
        REQUIRE_FALSE(p.existsAs<std::string>("parameter"));

        p.set("parameter", "test");

        REQUIRE(p.existsAs<std::string>("parameter"));
        REQUIRE(p.get<std::string>("parameter") == "test");
    }

    SECTION("Adding InputTag") {
        REQUIRE_FALSE(p.existsAs<InputTag>("parameter"));

        auto v = InputTag("module_name", "parameter");
        p.set("parameter", v);

        REQUIRE(p.existsAs<InputTag>("parameter"));
        REQUIRE(p.get<InputTag>("parameter") == v);
    }

    SECTION("Implicit int64_t cast") {
        REQUIRE_FALSE(p.existsAs<int64_t>("parameter"));

        p.set("parameter", 10);

        REQUIRE(p.existsAs<int64_t>("parameter"));
        REQUIRE(p.get<int64_t>("parameter") == 10);
    }

    SECTION("Implicit double cast") {
        REQUIRE_FALSE(p.existsAs<double>("parameter"));

        p.set("parameter", 10.f);

        REQUIRE(p.existsAs<double>("parameter"));
        REQUIRE(p.get<double>("parameter") == Approx(10));
    }

    SECTION("Names should return the list of keys") {
        p.set("p1", false);
        p.set("p2", true);

        auto names = p.getNames();
        REQUIRE(names.size() == 2);
        REQUIRE(names[0] == "p1");
        REQUIRE(names[1] == "p2");
    }
}
