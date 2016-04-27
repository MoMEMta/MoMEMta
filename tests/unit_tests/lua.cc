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
 * \brief Unit tests for the lua configuration file parser
 * \ingroup UnitTests
 */

#include <catch.hpp>

#include <iostream>
#include <string>
#include <vector>

#include <momemta/IOnModuleDeclared.h>
#include <momemta/ModuleFactory.h>
#include <momemta/InputTag.h>

#include <lua/utils.h>

void execute_string(std::shared_ptr<lua_State> L, const std::string& code) {
    if (luaL_dostring(L.get(), code.c_str())) {
        std::string error = lua_tostring(L.get(), -1);
        FAIL(error);
    }
}

class ModuleDeclaredMock: public IOnModuleDeclared {
    public:
        virtual void onModuleDeclared(const std::string& type, const std::string& name) override {
            modules.push_back({type, name});
        }

        std::vector<std::pair<std::string, std::string>> modules;
};

TEST_CASE("lua parsing utilities", "[lua]") {

    // Suppress log messages
    spdlog::set_level(spdlog::level::emerg);

    ModuleDeclaredMock moduleDeclared;
    REQUIRE(moduleDeclared.modules.empty());
    std::shared_ptr<lua_State> L = lua::init_runtime(&moduleDeclared);

    auto stack_size = lua_gettop(L.get());

    SECTION("custom functions") {
        execute_string(L, "load_modules('not_existing.so')");
        execute_string(L, "parameter('not_existing')");

        // Check that the CubaIndex() function returns the correct InputTag
        // and that the index gets correctly incremented at each call.
        execute_string(L, "index1 = CubaIndex()");
        lua_getglobal(L.get(), "index1");
        auto value = lua::to_any(L.get(), -1);
        REQUIRE( (boost::any_cast<InputTag>(value.first)).toString() == "cuba::ps_points/0");
        execute_string(L, "index2 = CubaIndex()");
        lua_getglobal(L.get(), "index2");
        value = lua::to_any(L.get(), -1);
        REQUIRE( (boost::any_cast<InputTag>(value.first)).toString() == "cuba::ps_points/1");
        lua_pop(L.get(), 2);
    }

    SECTION("defining modules") {
        execute_string(L, "Flatter.test = {}");
        REQUIRE(moduleDeclared.modules.size() == 1);
        REQUIRE(moduleDeclared.modules.back().first == "Flatter");
        REQUIRE(moduleDeclared.modules.back().second == "test");

        execute_string(L, "Flatter.test2 = {}");
        REQUIRE(moduleDeclared.modules.size() == 2);
        REQUIRE(moduleDeclared.modules.back().second == "test2");
    }

    SECTION("loading modules") {
        auto plugins = ModuleFactory::get().getPluginsList().size();

        // Fake call to load_modules
        lua_pushliteral(L.get(), "libempty_module.so");
        lua::load_modules(L.get());
        lua_pop(L.get(), 1);

        REQUIRE(ModuleFactory::get().getPluginsList().size() == plugins + 1);
    }

    SECTION("parsing values") {
        // Integer
        lua_pushinteger(L.get(), 42);
        auto value = lua::to_any(L.get(), -1);
        REQUIRE(value.first.type() == typeid(int64_t));
        REQUIRE(boost::any_cast<int64_t>(value.first) == 42);
        REQUIRE_FALSE(value.second);
        lua_pop(L.get(), 1);

        // Double
        lua_pushnumber(L.get(), 38.5);
        value = lua::to_any(L.get(), -1);
        REQUIRE(value.first.type() == typeid(double));
        REQUIRE(boost::any_cast<double>(value.first) == Approx(38.5));
        REQUIRE_FALSE(value.second);
        lua_pop(L.get(), 1);

        // Boolean
        lua_pushboolean(L.get(), true);
        value = lua::to_any(L.get(), -1);
        REQUIRE(value.first.type() == typeid(bool));
        REQUIRE(boost::any_cast<bool>(value.first) == true);
        REQUIRE_FALSE(value.second);
        lua_pop(L.get(), 1);

        // std::string
        lua_pushliteral(L.get(), "lua is fun");
        value = lua::to_any(L.get(), -1);
        REQUIRE(value.first.type() == typeid(std::string));
        REQUIRE(boost::any_cast<std::string>(value.first) == "lua is fun");
        REQUIRE_FALSE(value.second);
        lua_pop(L.get(), 1);

        // Double vector
        execute_string(L, "return {0.1, 0.2, 0.3}");
        value = lua::to_any(L.get(), -1);
        REQUIRE(value.first.type() == typeid(std::vector<double>));
        REQUIRE_FALSE(value.second);
        {
            auto v = boost::any_cast<std::vector<double>>(value.first);
            REQUIRE(v.size() == 3);
            REQUIRE(v[0] == Approx(0.1));
            REQUIRE(v[1] == Approx(0.2));
            REQUIRE(v[2] == Approx(0.3));
        }
        lua_pop(L.get(), 1);

        // Double vector with an integer inside
        execute_string(L, "return {0.1, 2, 0.3}");
        value = lua::to_any(L.get(), -1);
        REQUIRE(value.first.type() == typeid(std::vector<double>));
        REQUIRE_FALSE(value.second);
        {
            auto v = boost::any_cast<std::vector<double>>(value.first);
            REQUIRE(v.size() == 3);
            REQUIRE(v[0] == Approx(0.1));
            REQUIRE(v[1] == Approx(2));
            REQUIRE(v[2] == Approx(0.3));
        }
        lua_pop(L.get(), 1);

        // Integer vector
        execute_string(L, "return {1, 2, 3}");
        value = lua::to_any(L.get(), -1);
        REQUIRE(value.first.type() == typeid(std::vector<int64_t>));
        REQUIRE_FALSE(value.second);
        {
            auto v = boost::any_cast<std::vector<int64_t>>(value.first);
            REQUIRE(v.size() == 3);
            REQUIRE(v[0] == 1);
            REQUIRE(v[1] == 2);
            REQUIRE(v[2] == 3);
        }
        lua_pop(L.get(), 1);

        // Invalid array
        execute_string(L, "return {1, 'string', false}");
        REQUIRE_THROWS_AS(lua::to_any(L.get(), -1), lua::invalid_array_error);
        lua_pop(L.get(), 1);
    }

    SECTION("parsing lazy values") {

        // Setup global parameters table
        execute_string(L, "parameters = { top_mass = 173. }");

        SECTION("lazy function") {
            execute_string(L, "return parameter('top_mass')");
            auto value = lua::to_any(L.get(), -1);
            REQUIRE(value.second == true);
            REQUIRE(value.first.type() == typeid(lua::LazyFunction));
            auto fct = boost::any_cast<lua::LazyFunction>(value.first);
            auto fct_evaluated = fct();
            REQUIRE(fct_evaluated.type() == typeid(double));
            REQUIRE(boost::any_cast<double>(fct_evaluated) == Approx(173.));
            lua_pop(L.get(), 1);
        }

        SECTION("lazy function after modification of parameter") {
            execute_string(L, "return parameter('top_mass')");
            auto value = lua::to_any(L.get(), -1);
            REQUIRE(value.second == true);
            REQUIRE(value.first.type() == typeid(lua::LazyFunction));

            // Edit parameter
            lua_getglobal(L.get(), "parameters");
            lua_pushnumber(L.get(), 175.);
            lua_setfield(L.get(), -2, "top_mass");
            lua_pop(L.get(), 1);

            auto fct = boost::any_cast<lua::LazyFunction>(value.first);
            auto fct_evaluated = fct();
            REQUIRE(fct_evaluated.type() == typeid(double));
            REQUIRE(boost::any_cast<double>(fct_evaluated) == Approx(175.));
            lua_pop(L.get(), 1);
        }

        SECTION("lazy table field") {
            lua::LazyTableField lazy(L.get(), "parameters", "top_mass");

            SECTION("evaluation") {
                auto value = lazy();
                REQUIRE(value.type() == typeid(double));
                REQUIRE(boost::any_cast<double>(value) == Approx(173.));
            }

            SECTION("edition") {
                lazy.set(175.);

                auto value = lazy();
                REQUIRE(value.type() == typeid(double));
                REQUIRE(boost::any_cast<double>(value) == Approx(175.));
            }
        }
    }

    REQUIRE(stack_size == lua_gettop(L.get()));
}
