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

#include <momemta/Configuration.h>
#include <momemta/ModuleFactory.h>
#include <momemta/Module.h>
#include <momemta/ParameterSet.h>
#include <momemta/Pool.h>
#include <momemta/Solution.h>
#include <momemta/Types.h>
#include <momemta/Math.h>

#define N_PS_POINTS 5

// A mock of ParameterSet to change visibility of the constructor
class ParameterSetMock: public ParameterSet {
public:
    ParameterSetMock(const std::string& name):
            ParameterSet(name, name) {
        // Empty
    }

    void createMock(const std::string& name, const momemta::any& value) {
        create(name, value);
    }
};

std::shared_ptr<std::vector<double>> addPhaseSpacePoints(std::shared_ptr<Pool> pool) {
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

std::shared_ptr<std::vector<LorentzVector>> addInputParticles(std::shared_ptr<Pool> pool) {
    auto inputs = pool->put<std::vector<LorentzVector>>({"input", "particles"});

    // Some random massive 4-vectors to be used by the tests
    inputs->push_back( { 16.171895980835, -13.7919054031372, -3.42997527122497, 21.5293197631836 });
    inputs->push_back( { 71.3899612426758, 96.0094833374023, -77.2513122558594, 142.492813110352 });
    inputs->push_back( { -18.9018573760986, 10.0896110534668, -0.602926552295686, 21.4346446990967 });
    inputs->push_back( { -55.7908325195313, -111.59294128418, -122.144721984863, 174.66259765625 });

    // Some random massless 4-vectors to be used by the tests
    inputs->push_back( { 26.2347,76.1854,32.6172,86.9273 } );
    inputs->push_back( { -27.3647,30.2536,38.5939,56.1569 } );
    inputs->push_back( { -7.0817,-63.9966,-34.8497,73.2135 } );
    inputs->push_back( { 80.756,3.12662,33.0355,87.3078 } );

    return inputs;
}

TEST_CASE("Modules", "[modules]") {
    std::shared_ptr<Pool> pool(new Pool());
    std::shared_ptr<ParameterSetMock> parameters;

    auto createModule = [&pool, &parameters](const std::string& type) {
        Configuration::ModuleDecl module;
        module.name = type;
        module.type = type;
        module.parameters.reset(parameters->clone());

        auto result = momemta::ModuleFactory::get().create(type, pool, *parameters);
        REQUIRE(result.get());

        return result;
    };

    // Register phase space points, mocking what cuba would do
    auto ps_points = addPhaseSpacePoints(pool);
    // Put a few random input particles into the pool
    auto input_particles = addInputParticles(pool);

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

    SECTION("BlockA") {

        parameters.reset(new ParameterSetMock("BlockA"));

        double sqrt_s = 13000;

        parameters->set("energy", sqrt_s);

        parameters->set("p1", InputTag("input", "particles", 0));
        parameters->set("p2", InputTag("input", "particles", 1));
        parameters->set("branches", std::vector<InputTag>( { InputTag("input", "particles", 2) } ));

        Value<SolutionCollection> solutions = pool->get<SolutionCollection>({"BlockA", "solutions"});

        auto module = createModule("BlockA");

        REQUIRE(module->work() == Module::Status::OK);
        REQUIRE(solutions->size() == 1);

        for (const auto& solution: *solutions) {
            REQUIRE(solution.valid == true);

            LorentzVector test_pT = solution.values.at(0) + solution.values.at(1) + input_particles->at(2);
            REQUIRE(test_pT.Pt() == Approx(0));

            REQUIRE(solution.values.at(0).M() == Approx(input_particles->at(0).M()));
            REQUIRE(solution.values.at(0).Phi() == Approx(input_particles->at(0).Phi()));
            REQUIRE(solution.values.at(0).Theta() == Approx(input_particles->at(0).Theta()));

            REQUIRE(solution.values.at(1).M() == Approx(input_particles->at(1).M()));
            REQUIRE(solution.values.at(1).Phi() == Approx(input_particles->at(1).Phi()));
            REQUIRE(solution.values.at(1).Theta() == Approx(input_particles->at(1).Theta()));
        }
    }

    SECTION("BlockB") {

        parameters.reset(new ParameterSetMock("BlockB"));

        double sqrt_s = 13000;
        bool pT_is_met = false;

        parameters->set("energy", sqrt_s);
        parameters->set("pT_is_met", pT_is_met);

        parameters->set("s12", InputTag("mockS", "s12"));
        double s_12 = SQ(200);
        auto s12 = pool->put<double>({"mockS", "s12"});
        *s12 = s_12;

        parameters->set("p2", InputTag("input", "particles", 0));

        Value<SolutionCollection> solutions = pool->get<SolutionCollection>({"BlockB", "solutions"});

        auto module = createModule("BlockB");

        REQUIRE(module->work() == Module::Status::OK);
        REQUIRE(solutions->size() == 2);

        for (const auto& solution: *solutions) {
            REQUIRE(solution.valid == true);

            LorentzVector test_p12 = input_particles->at(0) + solution.values.at(0);

            REQUIRE(test_p12.M2() == Approx(s_12));
            REQUIRE(test_p12.Pt() == Approx(0));
            REQUIRE(solution.values.at(0).M() / solution.values.at(0).E() == Approx(0));
        }
    }

    SECTION("BlockC") {

        parameters.reset(new ParameterSetMock("BlockC"));

        double sqrt_s = 13000;
        bool pT_is_met = false;
        double m1 = 1.;

        parameters->set("energy", sqrt_s);
        parameters->set("pT_is_met", pT_is_met);
        parameters->set("m1", m1);

        parameters->set("s12", InputTag("mockS", "s12"));
        parameters->set("s123", InputTag("mockS", "s123"));

        double s_12 = SQ(80);
        double s_123 = SQ(173);

        auto s12 = pool->put<double>({"mockS", "s12"});
        auto s123 = pool->put<double>({"mockS", "s123"});

        *s12 = s_12;
        *s123 = s_123;

        parameters->set("p2", InputTag("input", "particles", 0));
        parameters->set("p3", InputTag("input", "particles", 5));
        parameters->set("branches", std::vector<InputTag>({InputTag("input", "particles", 2)}));

        Value<SolutionCollection> solutions = pool->get<SolutionCollection>({"BlockC", "solutions"});

        auto module = createModule("BlockC");

        REQUIRE(module->work() == Module::Status::OK);
        REQUIRE(solutions->size() >= 2);

        for (const auto& solution : *solutions) {
            REQUIRE(solution.valid == true);

            LorentzVector test_p12 = input_particles->at(0) + solution.values.at(0);
            LorentzVector test_p123 = solution.values.at(1) + test_p12;

            REQUIRE(test_p12.M2() == Approx(s_12));
            REQUIRE(test_p123.M2() == Approx(s_123));

            LorentzVector test_pT = test_p123 + input_particles->at(2);
            REQUIRE(test_pT.Pt() == Approx(0));

            REQUIRE(solution.values.at(0).M() == Approx(m1));
            REQUIRE(solution.values.at(1).M() / solution.values.at(1).E() == Approx(0.));
        }
    }

    SECTION("BlockD") {

        parameters.reset(new ParameterSetMock("BlockD"));

        double sqrt_s = 13000;
        bool pT_is_met = false;

        parameters->set("energy", sqrt_s);
        parameters->set("pT_is_met", pT_is_met);

        parameters->set("s13", InputTag("mockS", "s13"));
        parameters->set("s134", InputTag("mockS", "s134"));
        parameters->set("s25", InputTag("mockS", "s25"));
        parameters->set("s256", InputTag("mockS", "s256"));

        double s_13_25 = SQ(80);
        double s_134_256 = SQ(170);

        auto s13 = pool->put<double>({"mockS", "s13"});
        auto s134 = pool->put<double>({"mockS", "s134"});
        auto s25 = pool->put<double>({"mockS", "s25"});
        auto s256 = pool->put<double>({"mockS", "s256"});

        *s13 = s_13_25;
        *s134 = s_134_256;
        *s25 = s_13_25;
        *s256 = s_134_256;

        parameters->set("p3", InputTag("input", "particles", 0));
        parameters->set("p4", InputTag("input", "particles", 1));
        parameters->set("p5", InputTag("input", "particles", 2));
        parameters->set("p6", InputTag("input", "particles", 3));

        Value<SolutionCollection> solutions = pool->get<SolutionCollection>({"BlockD", "solutions"});

        auto module = createModule("BlockD");

        REQUIRE(module->work() == Module::Status::OK);
        REQUIRE(solutions->size() == 2);

        for (const auto& solution: *solutions) {
            REQUIRE(solution.valid == true);

            LorentzVector test_p13 = input_particles->at(0) + solution.values.at(0);
            LorentzVector test_p134 = input_particles->at(1) + test_p13;
            LorentzVector test_p25 = input_particles->at(2) + solution.values.at(1);
            LorentzVector test_p256 = input_particles->at(3) + test_p25;

            REQUIRE(test_p13.M2() == Approx(s_13_25));
            REQUIRE(test_p134.M2() == Approx(s_134_256));
            REQUIRE(test_p25.M2() == Approx(s_13_25));
            REQUIRE(test_p256.M2() == Approx(s_134_256));

            LorentzVector test_pT = test_p134 + test_p256;
            REQUIRE(test_pT.Pt() == Approx(0));

            REQUIRE(solution.values.at(0).M() / solution.values.at(0).E() == Approx(0));
            REQUIRE(solution.values.at(1).M() / solution.values.at(1).E() == Approx(0));
        }
    }

    SECTION("BlockE") {

        parameters.reset(new ParameterSetMock("BlockE"));

        double sqrt_s = 13000;
        parameters->set("energy", sqrt_s);

        parameters->set("s13", InputTag("mockS", "s13"));
        parameters->set("s24", InputTag("mockS", "s24"));
        parameters->set("s_hat", InputTag("mockS", "s_hat"));
        parameters->set("y_tot", InputTag("mockS", "y_tot"));

        double s_13 = SQ(100);
        double s_24 = SQ(200);
        double s_hat = SQ(500);
        double rap = 1.5;

        *pool->put<double>({"mockS", "s13"}) = s_13;
        *pool->put<double>({"mockS", "s24"}) = s_24;
        *pool->put<double>({"mockS", "s_hat"}) = s_hat;
        *pool->put<double>({"mockS", "y_tot"}) = rap;

        parameters->set("p3", InputTag("input", "particles", 0));
        parameters->set("p4", InputTag("input", "particles", 2));

        Value<SolutionCollection> solutions = pool->get<SolutionCollection>({"BlockE", "solutions"});

        auto module = createModule("BlockE");

        REQUIRE(module->work() == Module::Status::OK);
        REQUIRE(solutions->size() == 2);

        for (const auto& solution: *solutions) {
            REQUIRE(solution.valid == true);

            LorentzVector test_p13 = input_particles->at(0) + solution.values.at(0);
            LorentzVector test_p24 = input_particles->at(2) + solution.values.at(1);
            LorentzVector test_pT = test_p13 + test_p24;

            REQUIRE(test_p13.M2() == Approx(s_13));
            REQUIRE(test_p24.M2() == Approx(s_24));

            REQUIRE(test_pT.M2() == Approx(s_hat));
            REQUIRE(test_pT.Rapidity() == Approx(rap));
            REQUIRE(test_pT.Pt() == Approx(0));

            REQUIRE(solution.values.at(0).M() / solution.values.at(0).E() == Approx(0));
            REQUIRE(solution.values.at(1).M() / solution.values.at(1).E() == Approx(0));
        }
    }

    SECTION("BlockF") {

        parameters.reset(new ParameterSetMock("BlockF"));

        double sqrt_s = 13000;
        parameters->set("energy", sqrt_s);

        parameters->set("s13", InputTag("mockS", "s13"));
        parameters->set("s24", InputTag("mockS", "s24"));
        parameters->set("q1", InputTag("mockS", "q1"));
        parameters->set("q2", InputTag("mockS", "q2"));

        double s_13 = SQ(100);
        double s_24 = SQ(200);
        double q1 = 0.172;
        double q2 = 0.0086;

        *pool->put<double>({"mockS", "s13"}) = s_13;
        *pool->put<double>({"mockS", "s24"}) = s_24;
        *pool->put<double>({"mockS", "q1"}) = q1;
        *pool->put<double>({"mockS", "q2"}) = q2;

        parameters->set("p3", InputTag("input", "particles", 0));
        parameters->set("p4", InputTag("input", "particles", 2));

        Value<SolutionCollection> solutions = pool->get<SolutionCollection>({"BlockF", "solutions"});

        auto module = createModule("BlockF");

        REQUIRE(module->work() == Module::Status::OK);
        REQUIRE(solutions->size() == 2);

        for (const auto& solution: *solutions) {
            REQUIRE(solution.valid == true);

            LorentzVector test_p13 = input_particles->at(0) + solution.values.at(0);
            LorentzVector test_p24 = input_particles->at(2) + solution.values.at(1);
            LorentzVector test_pT = test_p13 + test_p24;

            REQUIRE(test_p13.M2() == Approx(s_13));
            REQUIRE(test_p24.M2() == Approx(s_24));

            REQUIRE(test_pT.E() == Approx(0.5 * (q1 + q2) * sqrt_s));
            REQUIRE(test_pT.Pz() == Approx(0.5 * (q1 - q2) * sqrt_s));
            REQUIRE(test_pT.Pt() == Approx(0));

            REQUIRE(solution.values.at(0).M() / solution.values.at(0).E() == Approx(0));
            REQUIRE(solution.values.at(1).M() / solution.values.at(1).E() == Approx(0));
        }
    }

    SECTION("BlockG") {

        parameters.reset(new ParameterSetMock("BlockG"));

        double sqrt_s = 13000;
        parameters->set("energy", sqrt_s);

        parameters->set("s12", InputTag("mockS", "s12"));
        parameters->set("s34", InputTag("mockS", "s34"));

        double s_12 = SQ(650);
        double s_34 = SQ(550);

        *pool->put<double>({"mockS", "s12"}) = s_12;
        *pool->put<double>({"mockS", "s34"}) = s_34;

        parameters->set("p1", InputTag("input", "particles", 4));
        parameters->set("p2", InputTag("input", "particles", 5));
        parameters->set("p3", InputTag("input", "particles", 6));
        parameters->set("p4", InputTag("input", "particles", 7));

        Value<SolutionCollection> solutions = pool->get<SolutionCollection>({"BlockG", "solutions"});

        auto module = createModule("BlockG");

        REQUIRE(module->work() == Module::Status::OK);
        REQUIRE(solutions->size() == 1);

        for (const auto& solution: *solutions) {
            REQUIRE(solution.valid == true);

            LorentzVector test_p12 = solution.values.at(0) + solution.values.at(1);
            LorentzVector test_p34 = solution.values.at(2) + solution.values.at(3);
            LorentzVector test_pT = test_p12 + test_p34;

            REQUIRE(test_p12.M2() == Approx(s_12));
            REQUIRE(test_p34.M2() == Approx(s_34));

            REQUIRE(test_pT.Pt() == Approx(0));

            REQUIRE(solution.values.at(0).M() / solution.values.at(0).E() == Approx(0));
            REQUIRE(solution.values.at(0).Phi() == Approx(input_particles->at(4).Phi()));
            REQUIRE(solution.values.at(0).Theta() == Approx(input_particles->at(4).Theta()));

            REQUIRE(solution.values.at(1).M() / solution.values.at(1).E() == Approx(0));
            REQUIRE(solution.values.at(1).Phi() == Approx(input_particles->at(5).Phi()));
            REQUIRE(solution.values.at(1).Theta() == Approx(input_particles->at(5).Theta()));

            REQUIRE(solution.values.at(2).M() / solution.values.at(2).E() == Approx(0));
            REQUIRE(solution.values.at(2).Phi() == Approx(input_particles->at(6).Phi()));
            REQUIRE(solution.values.at(2).Theta() == Approx(input_particles->at(6).Theta()));

            REQUIRE(solution.values.at(3).M() / solution.values.at(3).E() == Approx(0));
            REQUIRE(solution.values.at(3).Phi() == Approx(input_particles->at(7).Phi()));
            REQUIRE(solution.values.at(3).Theta() == Approx(input_particles->at(7).Theta()));

        }
    }

    SECTION("SecondaryBlockA") {

        parameters.reset(new ParameterSetMock("SecondaryBlockA"));

        double sqrt_s = 13000;
        parameters->set("energy", sqrt_s);

        parameters->set("s12", InputTag("mockS", "s12"));
        parameters->set("s123", InputTag("mockS", "s123"));
        parameters->set("s1234", InputTag("mockS", "s1234"));

        double s_12 = SQ(75);
        double s_123 = SQ(150);
        double s_1234 = SQ(350);

        *pool->put<double>({"mockS", "s12"}) = s_12;
        *pool->put<double>({"mockS", "s123"}) = s_123;
        *pool->put<double>({"mockS", "s1234"}) = s_1234;

        parameters->set("p2", InputTag("input", "particles", 0));
        parameters->set("p3", InputTag("input", "particles", 2));
        parameters->set("p4", InputTag("input", "particles", 1));

        Value<SolutionCollection> solutions = pool->get<SolutionCollection>({"SecondaryBlockA", "solutions"});

        auto module = createModule("SecondaryBlockA");

        REQUIRE(module->work() == Module::Status::OK);
        REQUIRE(solutions->size() == 2);

        for (const auto& solution: *solutions) {
            REQUIRE(solution.valid == true);

            LorentzVector test_p12 = input_particles->at(0) + solution.values.at(0);
            LorentzVector test_p123 = input_particles->at(2) + test_p12;
            LorentzVector test_p1234 = input_particles->at(1) + test_p123;

            REQUIRE(test_p12.M2() == Approx(s_12));
            REQUIRE(test_p123.M2() == Approx(s_123));
            REQUIRE(test_p1234.M2() == Approx(s_1234));

            REQUIRE(solution.values.at(0).M() / solution.values.at(0).E() == Approx(0));
        }
    }

    SECTION("SecondaryBlockB") {

        parameters.reset(new ParameterSetMock("SecondaryBlockB"));

        double sqrt_s = 13000;
        parameters->set("energy", sqrt_s);

        parameters->set("s12", InputTag("mockS", "s12"));
        parameters->set("s123", InputTag("mockS", "s123"));

        double s_12 = SQ(475.073);
        double s_123 = SQ(516.951);

        *pool->put<double>({"mockS", "s12"}) = s_12;
        *pool->put<double>({"mockS", "s123"}) = s_123;

        parameters->set("p1", InputTag("input", "particles", 0));
        parameters->set("p2", InputTag("input", "particles", 1));
        parameters->set("p3", InputTag("input", "particles", 2));

        Value<SolutionCollection> solutions = pool->get<SolutionCollection>({"SecondaryBlockB", "solutions"});

        auto module = createModule("SecondaryBlockB");

        REQUIRE(module->work() == Module::Status::OK);
        REQUIRE(solutions->size() == 1);

        for (const auto& solution: *solutions) {
            REQUIRE(solution.valid == true);

            LorentzVector test_p12 = input_particles->at(1) + solution.values.at(0);
            LorentzVector test_p123 = input_particles->at(2) + test_p12;

            REQUIRE(test_p12.M2() == Approx(s_12));
            REQUIRE(test_p123.M2() == Approx(s_123));

            REQUIRE(solution.values.at(0).M() == Approx(input_particles->at(0).M()));
            REQUIRE(solution.values.at(0).Phi() == Approx(input_particles->at(0).Phi()));
        }
    }

    SECTION("SecondaryBlockCD") {

        parameters.reset(new ParameterSetMock("SecondaryBlockCD"));

        double sqrt_s = 13000;
        parameters->set("energy", sqrt_s);

        parameters->set("s12", InputTag("mockS", "s12"));
        double s_12 = SQ(50);
        *pool->put<double>({"mockS", "s12"}) = s_12;

        parameters->set("p1", InputTag("input", "particles", 0));
        parameters->set("p2", InputTag("input", "particles", 2));

        Value<SolutionCollection> solutions = pool->get<SolutionCollection>({"SecondaryBlockCD", "solutions"});

        auto module = createModule("SecondaryBlockCD");

        REQUIRE(module->work() == Module::Status::OK);
        REQUIRE(solutions->size() == 1);

        for (const auto& solution: *solutions) {
            REQUIRE(solution.valid == true);

            LorentzVector test_p12 = input_particles->at(2) + solution.values.at(0);

            REQUIRE(test_p12.M2() == Approx(s_12));

            REQUIRE(solution.values.at(0).Phi() == Approx(input_particles->at(0).Phi()));
            REQUIRE(solution.values.at(0).Theta() == Approx(input_particles->at(0).Theta()));
        }
    }

    SECTION("SecondaryBlockE") {

        parameters.reset(new ParameterSetMock("SecondaryBlockE"));

        double sqrt_s = 13000;
        parameters->set("energy", sqrt_s);

        parameters->set("s12", InputTag("mockS", "s12"));
        parameters->set("s123", InputTag("mockS", "s123"));

        double s_12 = SQ(50);
        double s_123 = SQ(500);

        *pool->put<double>({"mockS", "s12"}) = s_12;
        *pool->put<double>({"mockS", "s123"}) = s_123;

        parameters->set("p1", InputTag("input", "particles", 0));
        parameters->set("p2", InputTag("input", "particles", 5));
        parameters->set("p3", InputTag("input", "particles", 3));

        Value<SolutionCollection> solutions = pool->get<SolutionCollection>({"SecondaryBlockE", "solutions"});

        auto module = createModule("SecondaryBlockE");

        REQUIRE(module->work() == Module::Status::OK);
        REQUIRE(solutions->size() == 2);

        for (const auto& solution: *solutions) {
            REQUIRE(solution.valid == true);

            LorentzVector test_p12 = solution.values.at(0) + solution.values.at(1);
            LorentzVector test_p123 = input_particles->at(3) + test_p12;

            REQUIRE(test_p12.M2() == Approx(s_12));
            REQUIRE(test_p123.M2() == Approx(s_123));

            REQUIRE(solution.values.at(0).M() == Approx(input_particles->at(0).M()));
            REQUIRE(solution.values.at(0).Phi() == Approx(input_particles->at(0).Phi()));
            REQUIRE(solution.values.at(0).Theta() == Approx(input_particles->at(0).Theta()));

            REQUIRE(solution.values.at(1).M() / solution.values.at(1).E() == Approx(0));
            REQUIRE(solution.values.at(1).Phi() == Approx(input_particles->at(5).Phi()));
            REQUIRE(solution.values.at(1).Theta() == Approx(input_particles->at(5).Theta()));
        }
    }
}
