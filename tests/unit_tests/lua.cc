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

#include <momemta/InputTag.h>
#include <momemta/ILuaCallback.h>
#include <momemta/Logging.h>
#include <momemta/ModuleFactory.h>
#include <momemta/ParameterSet.h>
#include <momemta/Path.h>

#include <lua/Path.h>
#include <lua/Types.h>
#include <lua/utils.h>

void execute_string(std::shared_ptr<lua_State> L, const std::string& code) {
    if (luaL_dostring(L.get(), code.c_str())) {
        std::string error = lua_tostring(L.get(), -1);
        FAIL(error);
    }
}

class LuaCallbackMock: public ILuaCallback {
    public:
        LuaCallbackMock(): n_dimensions(0) {}

        virtual void onModuleDeclared(const std::string& type, const std::string& name) override {
            modules.push_back({type, name});
        }

        virtual void onIntegrandDeclared(const InputTag& tag) {
            integrands.push_back(tag);
        }

        virtual void onNewPath(PathElementsPtr path) {
            paths.push_back(path);
        }

        virtual void addIntegrationDimension() {
            n_dimensions++;
        }

        std::vector<std::pair<std::string, std::string>> modules;
        std::vector<InputTag> integrands;
        std::vector<PathElementsPtr> paths;
        std::size_t n_dimensions;
};

// A small mock of LazyParameterSet to change visibility of the `freeze` function
class LazyParameterSetMock: public LazyParameterSet {
    using LazyParameterSet::LazyParameterSet;

    public:
        virtual void freeze() override {
            LazyParameterSet::freeze();
        }
};

TEST_CASE("lua parsing utilities", "[lua]") {

    // Suppress log messages
    logging::set_level(boost::log::trivial::fatal);

    LuaCallbackMock luaCallback;
    REQUIRE(luaCallback.modules.empty());
    std::shared_ptr<lua_State> L = lua::init_runtime(&luaCallback);

    auto stack_size = lua_gettop(L.get());

    SECTION("custom functions") {
        execute_string(L, "load_modules('not_existing.so')");
        execute_string(L, "parameter('not_existing')");

        // Check that the 'add_dimension()' function returns the correct InputTag
        // and that the index gets correctly incremented at each call.
        execute_string(L, "index1 = add_dimension()");
        lua_getglobal(L.get(), "index1");
        auto value = lua::to_any(L.get(), -1);
        REQUIRE( (boost::any_cast<InputTag>(value.first)).toString() == "cuba::ps_points/1");
        execute_string(L, "index2 = add_dimension()");
        lua_getglobal(L.get(), "index2");
        value = lua::to_any(L.get(), -1);
        REQUIRE( (boost::any_cast<InputTag>(value.first)).toString() == "cuba::ps_points/2");
        lua_pop(L.get(), 2);
        // 'add_dimension()' has been called twice, so we should have two dimension in the configuation:
        REQUIRE( luaCallback.n_dimensions == 2 );

        execute_string(L, "integrand('integrand1::output', 'integrand2::output')");
        REQUIRE( luaCallback.integrands.size() == 2 );
        REQUIRE( luaCallback.integrands.at(1).toString() == "integrand2::output" );
    }

    SECTION("defining modules") {
        execute_string(L, "BreitWignerGenerator.test = {}");
        REQUIRE(luaCallback.modules.size() == 1);
        REQUIRE(luaCallback.modules.back().first == "BreitWignerGenerator");
        REQUIRE(luaCallback.modules.back().second == "test");

        execute_string(L, "BreitWignerGenerator.test2 = {}");
        REQUIRE(luaCallback.modules.size() == 2);
        REQUIRE(luaCallback.modules.back().second == "test2");
    }

    SECTION("loading modules") {
        auto plugins = ModuleFactory::get().getPluginsList().size();

        execute_string(L, "load_modules('libempty_module.so')");

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

    SECTION("ParameterSet evaluation") {
        auto def = R"(test_table = {
    integer = 1,
    float = 10.,
    string = "test",
    inputtag = "module::parameter",
    vector = {0, 1, 2, 3}
})";

        execute_string(L, def);

        int type = lua_getglobal(L.get(), "test_table");
        REQUIRE(type == LUA_TTABLE);

        ParameterSet p;
        p.parse(L.get(), -1);

        REQUIRE(p.existsAs<int64_t>("integer"));
        REQUIRE(p.get<int64_t>("integer") == 1);

        REQUIRE(p.existsAs<double>("float"));
        REQUIRE(p.get<double>("float") == Approx(10.));

        REQUIRE(p.existsAs<std::string>("string"));
        REQUIRE(p.get<std::string>("string") == "test");

        auto i = InputTag("module", "parameter");
        REQUIRE(p.existsAs<InputTag>("inputtag"));
        REQUIRE(p.get<InputTag>("inputtag") == i);

        REQUIRE(p.existsAs<std::vector<int64_t>>("vector"));
        auto v = p.get<std::vector<int64_t>>("vector");
        REQUIRE(v.size() == 4);
        REQUIRE(v[0] == 0);
        REQUIRE(v[1] == 1);
        REQUIRE(v[2] == 2);
        REQUIRE(v[3] == 3);

        lua_pop(L.get(), 1);
    }

    SECTION("LazyParameterSet evaluation") {
        auto def = R"(test_table = {
    integer = 1,
    float = 10.,
    string = "test",
    inputtag = "module::parameter"
})";

        execute_string(L, def);

        int type = lua_getglobal(L.get(), "test_table");
        REQUIRE(type == LUA_TTABLE);

        LazyParameterSetMock p(L, "test_table");
        p.parse(L.get(), -1);

        auto f = p;
        f.freeze();

        REQUIRE(f.existsAs<int64_t>("integer"));
        REQUIRE(f.get<int64_t>("integer") == 1);

        REQUIRE(f.existsAs<double>("float"));
        REQUIRE(f.get<double>("float") == Approx(10.));

        REQUIRE(f.existsAs<std::string>("string"));
        REQUIRE(f.get<std::string>("string") == "test");

        auto i = InputTag("module", "parameter");
        REQUIRE(f.existsAs<InputTag>("inputtag"));
        REQUIRE(f.get<InputTag>("inputtag") == i);

        // Edit the parameter set, and refreeze

        // Change value
        p.set("integer", 10);

        // Change value AND type
        p.set("float", true);

        // Add new value
        p.set("new", 125.);

        f = p;
        f.freeze();

        REQUIRE(f.existsAs<int64_t>("integer"));
        REQUIRE(f.get<int64_t>("integer") == 10);

        REQUIRE(f.existsAs<bool>("float"));
        REQUIRE(f.get<bool>("float") == true);

        REQUIRE(f.existsAs<std::string>("string"));
        REQUIRE(f.get<std::string>("string") == "test");

        REQUIRE(f.existsAs<InputTag>("inputtag"));
        REQUIRE(f.get<InputTag>("inputtag") == i);

        REQUIRE(f.existsAs<double>("new"));
        REQUIRE(f.get<double>("new") == Approx(125));

        lua_pop(L.get(), 1);
    }

    SECTION("LazyParameterSet with non-existing table") {

        int type = lua_getglobal(L.get(), "test_table");
        lua_pop(L.get(), 1);
        REQUIRE(type == LUA_TNIL);

        LazyParameterSetMock p(L, "test_table");

        // Table must not exist
        type = lua_getglobal(L.get(), "test_table");
        lua_pop(L.get(), 1);
        REQUIRE(type == LUA_TNIL);

        p.set("key", "value");

        // Table must have been created
        type = lua_getglobal(L.get(), "test_table");
        lua_pop(L.get(), 1);
        REQUIRE(type == LUA_TTABLE);

        p.freeze();

        REQUIRE(p.existsAs<std::string>("key"));
        REQUIRE(p.get<std::string>("key") == "value");
    }

    SECTION("Path") {
        auto def = R"(path = Path("a", "b", "c"))";
        execute_string(L, def);

        auto type = lua_getglobal(L.get(), "path");
        REQUIRE(type == LUA_TUSERDATA);

        std::string type_name = get_custom_type_name(L.get(), -1);
        REQUIRE(type_name == LUA_PATH_TYPE_NAME);

        PathElementsPtr path = lua::path_get(L.get(), -1);
        REQUIRE(path != nullptr);

        REQUIRE(path->elements.size() == 3);
        REQUIRE(path->elements[0] == "a");
        REQUIRE(path->elements[1] == "b");
        REQUIRE(path->elements[2] == "c");

        lua_pop(L.get(), 1);
    }

    SECTION("Path to boost::any") {
        auto def = R"(path = Path("a"))";
        execute_string(L, def);

        auto type = lua_getglobal(L.get(), "path");
        REQUIRE(type == LUA_TUSERDATA);

        auto path = get_custom_type_ptr(L.get(), -1);
        REQUIRE(path.type() == typeid(Path));

        lua_pop(L.get(), 1);
    }

    REQUIRE(stack_size == lua_gettop(L.get()));
}
