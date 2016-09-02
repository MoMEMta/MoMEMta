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
 * \brief Unit tests for the memory Pool
 * \sa Pool
 * \ingroup UnitTests
 */

#include <catch.hpp>

#include <momemta/Pool.h>

TEST_CASE("memory pool", "[pool]") {
    std::unique_ptr<Pool> pool(new Pool());
    pool->current_module("unit_tests");

    SECTION("Put and get") {
        InputTag tag("module", "parameter");

        auto ptr = pool->put<double>(tag);
        REQUIRE(ptr.get());

        *ptr = 12.5;

        auto read = pool->get<double>(tag);
        REQUIRE(*read == Approx(12.5));
    }

    SECTION("Get should allocate memory") {
        InputTag tag("module", "parameter");

        auto value = pool->get<double>(tag);
        REQUIRE(value.get());

        auto ptr = pool->put<double>(tag);
        *ptr = 12.5;

        REQUIRE(*value == Approx(12.5));
    }

    SECTION("Put is allowed only once") {
        InputTag tag("module", "parameter");

        auto value = pool->put<double>(tag);
        REQUIRE(value.get());

        CHECK_THROWS_AS(pool->put<double>(tag), Pool::duplicated_tag_error);
    }

    SECTION("Indexed InputTag") {
        InputTag tag("module", "parameter");
        InputTag indexed_tag("module", "parameter", 1);

        auto ptr = pool->put<std::vector<double>>(tag);
        REQUIRE(ptr.get());

        ptr->push_back(0);
        ptr->push_back(1);

        auto value = pool->get<double>(indexed_tag);
        REQUIRE(*value == Approx(1));
    }

    SECTION("Get should allocate a vector for indexed InputTag") {
        InputTag tag("module", "parameter", 1);

        auto value = pool->get<double>(tag);

        auto ptr = pool->put<std::vector<double>>(tag);
        ptr->push_back(0);
        ptr->push_back(1);

        REQUIRE(*value == Approx(1));
    }
}