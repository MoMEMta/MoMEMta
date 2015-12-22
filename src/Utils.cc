#include <Utils.h>

#include <iostream>

using namespace std;

bool solveQuadratic(const double a, const double b, const double c, std::vector<double>& roots, bool verbose){

    if(!a){
        if(!b){
            cout << "No solution to equation " << a << " x^2 + " << b << " x + " << c << endl << endl;
            return false;
        }
        roots.push_back(-c/b);
        if(verbose)
            cout << "Solution of " << b << " x + " << c << ": " << roots[0] << ", test = " << b*roots[0] + c << endl << endl;
        return true;
    }

    const double rho = SQ(b) - 4.*a*c;

    if(rho >= 0.){
        if(b == 0.){
            roots.push_back( sqrt(rho)/(2.*a) );
            roots.push_back( -sqrt(rho)/(2.*a) );
        }else{
            const double x = -0.5*(b + sign(b)*sqrt(rho));
            roots.push_back(x/a);
            roots.push_back(c/x);
        }
        if(verbose){
            cout << "Solutions of " << a << " x^2 + " << b << " x + " << c << ":" << endl;
            for(unsigned short i=0; i<roots.size(); i++)
                cout << "x" << i << " = " << roots[i] << ", test = " << a*SQ(roots[i]) + b*roots[i] + c << endl;
            cout << endl;
        }
        return true;
    }else{
        if(verbose)
            cout << "No real solutions to " << a << " x^2 + " << b << " x + " << c << endl << endl;
        return false;
    }
}

bool solveCubic(const double a, const double b, const double c, const double d, std::vector<double>& roots, bool verbose){

    if(a == 0)
        return solveQuadratic(b, c, d, roots, verbose);

    const double an = b/a;
    const double bn = c/a;
    const double cn = d/a;

    const double Q = SQ(an)/9. - bn/3.;
    const double R = CB(an)/27. - an*bn/6. + cn/2.;

    if( SQ(R) < CB(Q) ){
        const double theta = acos( R/sqrt(CB(Q)) )/3.;

        roots.push_back( -2. * sqrt(Q) * cos(theta) - an/3. );
        roots.push_back( -2. * sqrt(Q) * cosXpm2PI3(theta, 1.) - an/3. );
        roots.push_back( -2. * sqrt(Q) * cosXpm2PI3(theta, -1.) - an/3. );
    }else{
        const double A = - sign(R) * cbrt( abs(R) + sqrt( SQ(R) - CB(Q) ) );

        double B;

        if(A == 0.)
            B = 0.;
        else
            B = Q/A;

        const double x = A + B - an/3.;

        roots.push_back(x);
        roots.push_back(x);
        roots.push_back(x);
    }

    if(verbose){
        cout << "Solutions of " << a << " x^3 + " << b << " x^2 + " << c << " x + " << d << ":" << endl;
        for(unsigned short i=0; i<roots.size(); i++)
            cout << "x" << i << " = " << roots[i] << ", test = " << a*CB(roots[i]) + b*SQ(roots[i]) + c*roots[i] + d << endl;
        cout << endl;
    }

    return true;
}

bool solveQuartic(const double a, const double b, const double c, const double d, const double e, std::vector<double>& roots, bool verbose){

    if(!a)
        return solveCubic(b, c, d, e, roots, verbose);

    if(!b && !c && !d){
        roots.push_back(0.);
        roots.push_back(0.);
        roots.push_back(0.);
        roots.push_back(0.);
    }else{
        const double an = b/a;
        const double bn = c/a - (3./8.) * SQ(b/a);
        const double cn = CB(0.5*b/a) - 0.5*b*c/SQ(a) + d/a;
        const double dn = -3.*QU(0.25*b/a) + e/a - 0.25*b*d/SQ(a) + c*SQ(b/4.)/CB(a);

        std::vector<double> res;
        solveCubic(1., 2.*bn, SQ(bn) - 4.*dn, -SQ(cn), res, verbose);
        short pChoice = -1;

        for(unsigned short i = 0; i<res.size(); ++i){
            if(res[i] > 0){
                pChoice = i;
                break;
            }
        }

        if(pChoice < 0){
            if(verbose)
                cout << "No real solution to " << a << " x^4 + " << b << " x^3 + " << c << " x^2 + " << d << " x + " << e << " (no positive root for the resolvent cubic)." << endl << endl;
            return false;
        }

        const double p = sqrt(res[pChoice]);
        solveQuadratic(p, SQ(p), 0.5*( p*(bn + res[pChoice]) - cn ), roots, verbose);
        solveQuadratic(p, -SQ(p), 0.5*( p*(bn + res[pChoice]) + cn ), roots, verbose);

        for(unsigned short i = 0; i<roots.size(); ++i)
            roots[i] -= an/4.;
    }

    size_t nRoots = roots.size();

    if(verbose){
        if(nRoots){
            cout << "Solutions of " << a << " x^4 + " << b << " x^3 + " << c << " x^2 + " << d << " x + " << e << ":" << endl;
            for(unsigned short i=0; i<nRoots; i++)
                cout << "x" << i << " = " << roots[i] << ", test = " << a*QU(roots[i]) + b*CB(roots[i]) + c*SQ(roots[i]) + d*roots[i] + e << endl;
            cout << endl;
        }else{
            cout << "No real solution to " << a << " x^4 + " << b << " x^3 + " << c << " x^2 + " << d << " x + " << e << endl << endl;
        }
    }

    return nRoots > 0;
}

bool solve2Quads(const double a20, const double a02, const double a11, const double a10, const double a01, const double a00,
        const double b20, const double b02, const double b11, const double b10, const double b01, const double b00,
        std::vector<double>& E1, std::vector<double>& E2,
        bool verbose){

    // The procedure used in this function relies on a20 != 0 or b20 != 0
    if(a20 == 0. && b20 == 0.){

        if(a02 != 0. || b02 != 0.){
            // Swapping E1 <-> E2 should suffice!
            return solve2Quads(a02, a20, a11, a01, a10, a00,
                    b02, b20, b11, b01, b10, b00,
                    E2, E1, verbose);
        }else{
            return solve2QuadsDeg(a11, a10, a01, a00,
                    b11, b10, b01, b00,
                    E1, E2, verbose);
        }

    }

    const double alpha = b20*a02-a20*b02;
    const double beta = b20*a11-a20*b11;
    const double gamma = b20*a10-a20*b10;
    const double delta = b20*a01-a20*b01;
    const double omega = b20*a00-a20*b00;

    const double a = a20*SQ(alpha) + a02*SQ(beta) - a11*alpha*beta;
    const double b = 2.*a20*alpha*delta - a11*( alpha*gamma + delta*beta ) - a10*alpha*beta + 2.*a02*beta*gamma + a01*SQ(beta);
    const double c = a20*SQ(delta) + 2.*a20*alpha*omega - a11*( delta*gamma + omega*beta ) - a10*( alpha*gamma + delta*beta )
        + a02*SQ(gamma) + 2.*a01*beta*gamma + a00*SQ(beta);
    const double d = 2.*a20*delta*omega - a11*omega*gamma - a10*( delta*gamma + omega*beta ) + a01*SQ(gamma) + 2.*a00*beta*gamma;
    const double e = a20*SQ(omega) - a10*omega*gamma + a00*SQ(gamma);

    solveQuartic(a, b, c, d, e, E2, verbose);

    for(unsigned short i = 0; i < E2.size(); ++i){

        const double e2 = E2[i];

        if(beta*e2 + gamma != 0.){
            // Everything OK

            const double e1 = -(alpha * SQ(e2) + delta*e2 + omega)/(beta*e2 + gamma);
            E1.push_back(e1);

        }else if(alpha*SQ(e2) + delta*e2 + omega == 0.){
            // Up to two solutions for e1

            std::vector<double> e1;

            if( !solveQuadratic(a20, a11*e2 + a10, a02*SQ(e2) + a01*e2 + a00, e1, verbose) ){

                if( !solveQuadratic(b20, b11*e2 + b10, b02*SQ(e2) + b01*e2 + b00, e1, verbose) ){
                    cout << "Error in solve2Quads: there should be at least one solution for e1!" << endl;
                    E1.clear();
                    E2.clear();
                    return false;
                }

            }else{
                // We have either a double, or two roots for e1
                // In this case, e2 must be twice degenerate!
                // Since in E2 degenerate roots are grouped, E2[i+1] shoud exist and be equal to e2
                // We then go straight for i+2.

                if(i < E2.size() - 1){

                    if(e2 != E2[i+1]){
                        cout << "Error in solve2Quads: if there are two solutions for e1, e2 should be degenerate!" << endl;
                        E1.clear();
                        E2.clear();
                        return false;
                    }

                    E1.push_back(e1[0]);
                    E1.push_back(e1[1]);
                    ++i;
                    continue;

                }else{
                    cout << "Error in solve2Quads: if there are two solutions for e1, e2 should be degenerate!" << endl;
                    E1.clear();
                    E2.clear();
                    return false;
                }

            }

        }else{
            // There is no solution given this e2
            E2.erase(E2.begin() + i);
            --i;
        }

    }

    return true;
}

bool solve2QuadsDeg(const double a11, const double a10, const double a01, const double a00,
        const double b11, const double b10, const double b01, const double b00,
        vector<double>& E1, vector<double>& E2,
        bool verbose){

    if(a11 == 0. && b11 == 0.)
        return solve2Linear(a10, a01, a00, b10, b01, b00, E1, E2, verbose);

    bool result = solveQuadratic(a11*(b11*a10-a11*b10),
            a01*(b11*a10-a11*b10) - a01*(b11*a01-a11*b01) + a11*(b11*a00-a11*b00),
            a01*(b11*a00-a11*b00),
            E1, verbose);

    if(!result){
        if(verbose){
            cout << "No solution to the system:" << endl;
            cout << " " << a11 << "*E1*E2 + " << a10 << "*E1 + " << a01 << "*E2 + " << a00 << " = 0" << endl;
            cout << " " << b11 << "*E1*E2 + " << b10 << "*E1 + " << b01 << "*E2 + " << b00 << " = 0" << endl;
            cout << endl;
        }
        return false;
    }

    for(unsigned short i=0; i<E1.size(); ++i){

        double denom = a11*E1[i] + a01;

        if(denom != 0){
            E2.push_back( -(a10*E1[i] + a00)/denom );

        }else{
            denom = b11*a01 - a11*b01;
            if(denom != 0.){
                E2.push_back( -( (b11*a10 - a11*b10)*E1[i] + b11*a00 - a11*b00 )/denom );
            }else{
                denom = b11*E1[i] + b01;
                if(denom != 0.){
                    E2.push_back( -(b10*E1[i] + b00)/denom );
                }else{
                    E1.erase(E1.begin() + i);
                    --i;
                }
            }
        }

    }

    if(verbose){
        cout << "Solutions to the system:" << endl;
        cout << " " << a11 << "*E1*E2 + " << a10 << "*E1 + " << a01 << "*E2 + " << a00 << " = 0" << endl;
        cout << " " << b11 << "*E1*E2 + " << b10 << "*E1 + " << b01 << "*E2 + " << b00 << " = 0" << endl;
        for(unsigned short i=0; i<E1.size(); ++i)
            cout << "  E1 = " << E1[i] << ", E2 = " << E2[i] << endl;
        cout << endl;
    }

    return E1.size();
}

bool solve2Linear(const double a10, const double a01, const double a00,
        const double b10, const double b01, const double b00,
        std::vector<double>& E1, std::vector<double>& E2, bool verbose){

    const double det = a10*b01 - b10*a01;

    if(det == 0.){
        if(a00 != 0. || b00 != 0.){
            if(verbose){
                cout << "No solution to the system:" << endl;
                cout << " " << a10 << "*E1 + " << a01 << "*E2 + " << a00 << " = 0" << endl;
                cout << " " << b10 << "*E1 + " << b01 << "*E2 + " << b00 << " = 0" << endl;
                cout << endl;
            }
            return false;
        }else{
            cout << "Error in solve2Linear: indeterminate system:" << endl;
            cout << " " << a10 << "*E1 + " << a01 << "*E2 + " << a00 << " = 0" << endl;
            cout << " " << b10 << "*E1 + " << b01 << "*E2 + " << b00 << " = 0" << endl;
            cout << endl;
            return false;
        }
    }

    const double e2 = (b10*a00-a10*b00)/det;
    E2.push_back(e2);
    double e1;
    if(a10 == 0.)
        e1 = -(b00 + b01*e2)/b10;
    else
        e1 = -(a00 + a01*e2)/a10;
    E1.push_back(e1);

    if(verbose){
        cout << "Solution to the system:" << endl;
        cout << " " << a10 << "*E1 + " << a01 << "*E2 + " << a00 << " = 0" << endl;
        cout << " " << b10 << "*E1 + " << b01 << "*E2 + " << b00 << " = 0" << endl;
        cout << "  E1 = " << e1 << ", E2 = " << e2 << endl << endl;
    }

    return true;
}

double BreitWigner(const double s, const double m, const double g){
    /*double ga = sqrt(m*m*(m*l+g*g));
      double k = 2*sqrt(2)*m*g*ga/(TMath::Pi()*sqrt(m*m+ga));*/
    double k = m*g;

    //cout << "BW(" << s << "," << m << "," << g << ")=" << k/(pow(s-m*m,2.) + pow(m*g,2.)) << endl;

    return k/(pow(s-m*m,2.) + pow(m*g,2.));
}


