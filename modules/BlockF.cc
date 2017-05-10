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

#include <momemta/Logging.h>
#include <momemta/Module.h>
#include <momemta/ParameterSet.h>
#include <momemta/Solution.h>
#include <momemta/Types.h>
#include <momemta/Math.h>

/*! \brief \f$\require{cancel}\f$ Final (main) Block F, describing \f$q_1 q_2 \to X + s_{13} (\to \cancel{p_1} p_3) + s_{24} (\to \cancel{p_2} p_4)\f$
 *
 * Final (main) Block F on \f$q_1 q_2 \to X + s_{13} + s_{24} \to X + p_1 p_2 p_3 p_4\f$,  
 * where \f$q_1\f$ and \f$q_2\f$ are Bjorken fractions, \f$s_{13}\f$ and \f$s_{24}\f$ are particles
 * decaying respectively into \f$p_1\f$ (invisible particle) and \f$p_3\f$ (visible particle),
 * and \f$p_2\f$ (invisible particle) and \f$p_4\f$ (visible particle).
 * 
 * This Block addresses the change of variables needed to pass from the standard phase-space
 * parametrization to the \f$\frac{1}{16\pi^2 E_1 E_2} dq_{1} dq_{2} ds_{13} d_s{24}  \times J\f$ parametrization.
 * 
 * The integration is performed over \f$q_{1}\f$, \f$q_{2}\f$, \f$s_{13}\f$ and \f$s_{24}\f$
 * with \f$p_3\f$ and \f$p_4\f$ as inputs. Per integration point, 
 * the LorentzVectors of the invisible particle, \f$p_1\f$ and \f$p_2\f$,
 * are computed  based on this set of equations:
 *
 * - \f$s_{13} = (p_1 + p_3)^2\f$
 * - \f$s_{24} = (p_2 + p_4)^2\f$
 * - Conservation of momentum (with \f$p^{vis}\f$ the total momentum of visible particles):
 *  - \f$p_{1x} + p_{2x} = - p_{x}^{vis}\f$
 *  - \f$p_{1y} + p_{2y} = - p_{y}^{vis}\f$
 * - \f$p_{1z} + p_{2z} = s^{1/2} (q_1 - q_2)/2 - p_z^{vis}\f$
 * - \f$E_{1} + E_{2} = s^{1/2} (q_1 + q_2)/2 - E^{vis}\f$
 * - \f$p_1^2 = m_1^2\f$
 * - \f$p_2^2 = m_2^2\f$
 *
 * The observed MET is not used in this block since to reconstruct the
 * neutrinos the system requires as input the total 4-momentum of the 
 * visible objects (energy and longitudinal momentum included).
 *
 * Up to 2 (\f$p_1\f$, \f$p_2\f$) solutions are possible.
 *
 *
 * ### Integration dimension
 *
 * This module requires **2** phase-space points.
 *
 * ### Global parameters
 *
 *   | Name | Type | %Description |
 *   |------|------|-------------|
 *   | `energy` | double | Collision energy |
 *
 * ### Parameters
 *
 *   | Name | Type | %Description |
 *   | `m1` <br /> `m2` | double, default 0 | Masses of the invisible particles \f$p_1\f$ and \f$p_2\f$ |
 *
 * ### Inputs
 *
 *   | Name | Type | %Description |
 *   |------|------|-------------|
 *   | `q1` <br /> `q2` | double | Bjorken fractions. These are the dimensions of integration coming from CUBA as phase-space points |
 *   | `s13` <br/> `s24` | double | Squared invariant masses of the two propagators, used to reconstruct the event according to the above method. Typically coming from a BreitWignerGenerator module. |
 *   | `p3` <br/> `p4` | LorentzVector | LorentzVectors of the two particles used to reconstruct the event according to the above method. |
 *   | `branches` | vector(LorentzVector) | LorentzVectors of all the other particles in the event, used to compute \f$p^{vis}\f$ and check if the solutions are physical. |
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|-------------|
 *   | `solutions` | vector(Solution) | Solutions of the change of variable. Each solution embed  the LorentzVectors of the invisible particles (\f$p1\f$, \f$p2\f$ in this case) and the associated jacobian. These solutions should be fed as input to the Looper module. |
 *
 * \note This block has been validated and is safe to use.
 *
 * \sa Looper module to loop over the solutions of this Block
 *
 * \ingroup modules
 */

class BlockF: public Module {
    public:

  BlockF(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName()) {

            m_ps_point1 = get<double>(parameters.get<InputTag>("q1"));
            m_ps_point2 = get<double>(parameters.get<InputTag>("q2"));

            m1 = parameters.get<double>("m1", 0.);
            m2 = parameters.get<double>("m2", 0.);
            
            sqrt_s = parameters.globalParameters().get<double>("energy");

            s13 = get<double>(parameters.get<InputTag>("s13"));
            s24 = get<double>(parameters.get<InputTag>("s24"));
	    
            p3 = get<LorentzVector>(parameters.get<InputTag>("p3"));
            p4 = get<LorentzVector>(parameters.get<InputTag>("p4"));

            if (parameters.exists("branches")) {
                auto branches_tags = parameters.get<std::vector<InputTag>>("branches");
                for (auto& t: branches_tags)
                    m_branches.push_back(get<LorentzVector>(t));
            }
        };

        virtual Status work() override {

            solutions->clear();

            const double sq_m1 = SQ(m1);
            const double sq_m2 = SQ(m2);
            const double sq_m3 = p3->M2();
            const double sq_m4 = p4->M2();
            
            // Don't spend time on unphysical part of phase-space
            if (sq_m1 + sq_m3 >= *s13 || sq_m2 + sq_m4 >= *s24 || *s13 + *s24 > SQ(sqrt_s))
                return Status::NEXT;
            
            const double p3x = p3->Px();
            const double p3y = p3->Py();
            const double p3z = p3->Pz();
            const double E3 = p3->E();
 
            const double p4x = p4->Px();
            const double p4y = p4->Py();
            const double p4z = p4->pz();
            const double E4 = p4->E();
 
            // Total visible momentum
            LorentzVector pb = *p3 + *p4;
            for (size_t i = 0; i < m_branches.size(); i++) {
                pb += *m_branches[i];
            }
            
            const double Eb = pb.E();
            const double pbx = pb.Px();
            const double pby = pb.Py();
            const double pbz = pb.Pz();
 
            const double q1 = *m_ps_point1;
            const double q2 = *m_ps_point2;

            const double Etot = sqrt_s * (q1 + q2) / 2 - Eb;
            const double ptotz = sqrt_s * (q1 - q2) / 2 - pbz;
            
            const double X = 0.5 * (- sq_m1 - sq_m3 + *s13);
            const double Y = 0.5 * (- sq_m2 - sq_m4 + *s24);


            /* Solve the linear system:
             * p1x + p2x = - pbx
             * p1y + p2y = - pby
             * p1z + p2z = ptotz
             * p3x p1x + p3y p1y + p3z p1z = - X + E3 E1
             * p4x p2x + p4z p2z = - Y + E4 E2 -p4y p2y
             *
             * The solutions are parameterised by E2 and p2y:
             * p1x = A1x E2 + B1x p2y + C1x
             * p1y = - p2y - pby
             * p1z = A1z E2 + B1z p2y + C1z
             * p2x = - A1x E2 - B1x p2y - (C1x + pbx)
             * p2z = - A1z E2 - B1z p2y - (C1z - ptotz)
             *
             * where one has used that E1 = Etot - E2
             */
            
            const double den = p3z * p4x - p3x * p4z;

            const double A1x = - (E4 * p3z - E3 * p4z) / den;
            const double B1x = (p3z * p4y - p3y * p4z) / den;
            const double C1x = - (p4z * (E3 * Etot - p3z * ptotz + p3y * pby - X) - p3z * (Y - p4x * pbx)) / den;
            
            const double A1z = (E4 * p3x - E3 * p4x) / den;
            const double B1z = (p3y * p4x - p3x * p4y) / den;
            const double C1z = (p4x * (E3 * Etot + p3y * pby + p3x * pbx - X) - p3x * (Y + p4z * ptotz)) / den;


            /* Now, insert those expressions into the mass-shell conditions for p1 and p2:
             * (Etot - E2)^2 - p1x^2 - p1y^2 - p1z^2 - m1^2 = 0 (1)
             * E2^2 - p2x^2 - p2y^2 - p2z^2 - m2^2 = 0          (2)
             *
             * Using the above, (1) is written as:
             * a20 E2^2 + a02 p2y^2 + a11 E2 p2y + a10 E2 + a01 p2y + a00 = 0
             *
             * From (2)-(1), is it possible to write E2 = a p2y + b. This is then inserted into (1),
             * yielding a quadratic equation in p2y only.
             */

            const double fac = 2 * (A1x * pbx - A1z * ptotz - Etot);
            const double a = - 2 * (B1x * pbx - B1z * ptotz - pby) / fac;
            const double b = - (SQ(Etot) + pow(C1x + pbx, 2) + pow(C1z - ptotz, 2) + sq_m2 - SQ(C1x) - SQ(pby) - SQ(C1z) - sq_m1) / fac;

            const double a20 = 1 - SQ(A1x) - SQ(A1z);
            const double a02 = - (SQ(B1x) + SQ(B1z) + 1);
            const double a11 = - 2 * (A1x * B1x + A1z * B1z);
            const double a10 = - 2 * (A1x * C1x + A1z * C1z + Etot);
            const double a01 = - 2 * (B1x * C1x + B1z * C1z + pby);
            const double a00 = SQ(Etot) - (SQ(C1x) + SQ(C1z) + SQ(pby) + sq_m1);
          
            std::vector<double> p2y_sol;
            const bool foundSolution = solveQuadratic(a02 + SQ(a) * a20 + a * a11, 
                                                2 * a * b * a20 + b * a11 + a01 + a * a10,
                                                SQ(b) * a20 + b * a10 + a00,
                                                p2y_sol);

            if (!foundSolution)
                return Status::NEXT;

            for (const double p2y: p2y_sol) {
                const double E2 = a * p2y + b;

                if (E2 <= 0)
                    continue;

                const double E1 = Etot - E2;
                
                if (E1 <= 0)
                    continue;
                
                const double p1x = A1x * E2 + B1x * p2y + C1x;
                const double p1y = - p2y - pby;
                const double p1z = A1z * E2 + B1z * p2y + C1z;
                const LorentzVector p1(p1x, p1y, p1z, E1);

                const double p2x = - p1x - pbx;
                const double p2z = - p1z + ptotz;
                const LorentzVector p2(p2x, p2y, p2z, E2);

                // Check if solutions are physical
                const LorentzVector tot = p1 + p2 + pb;
                const double q1Pz = std::abs(tot.Pz() + tot.E()) / 2.;
                const double q2Pz = std::abs(tot.Pz() - tot.E()) / 2.;

                if (q1Pz > sqrt_s/2 || q2Pz > sqrt_s/2)
                    continue;

                const double jacobian = 1. / (64 * SQ(M_PI) * std::abs(E4*(p1z*p2y*p3x - p1y*p2z*p3x - p1z*p2x*p3y + p1x*p2z*p3y + p1y*p2x*p3z - p1x*p2y*p3z) +  E2*p1z*p3y*p4x - E1*p2z*p3y*p4x - E2*p1y*p3z*p4x + E1*p2y*p3z*p4x - E2*p1z*p3x*p4y + E1*p2z*p3x*p4y +  E2*p1x*p3z*p4y - E1*p2x*p3z*p4y + (E2*p1y*p3x - E1*p2y*p3x - E2*p1x*p3y + E1*p2x*p3y)*p4z + E3*(-(p1z*p2y*p4x) + p1y*p2z*p4x + p1z*p2x*p4y - p1x*p2z*p4y - p1y*p2x*p4z + p1x*p2y*p4z)));

                Solution s { {p1, p2}, jacobian, true };
                solutions->push_back(s);
            }

            return solutions->size() > 0 ? Status::OK : Status::NEXT;
        }
    
    private:
        double sqrt_s;

        // Inputs
        Value<double> s13;
        Value<double> s24;
        Value<double> m_ps_point1;
        Value<double> m_ps_point2;
        double m1;
        double m2;
        Value<LorentzVector> p3, p4;
        std::vector<Value<LorentzVector>> m_branches;

        // Outputs
        std::shared_ptr<SolutionCollection> solutions = produce<SolutionCollection>("solutions");
};
REGISTER_MODULE(BlockF);
