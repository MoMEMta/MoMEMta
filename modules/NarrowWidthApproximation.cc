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

/** \brief Use the Narrow Width Approximation (NWA) to reduce the dimensionality of the integration.
 *
 *  It is possible to reduce the number of integrated dimensions by fixing the mass of one of the propagators
 *  one is integrating over to the observed mass of the corresponding particle. 
 *  This approximation is exact in the limit \f$\Gamma/m \to 0\f$.
 *
 *  To implement this approximation in MoMEMta, you need to change the way the Block is fed the `s` (propagator mass squared) variable: 
 *  Instead of retrieving it from the `BreitWignerGenerator` module (which adds a dimension to carry out integration over
 *  the propagator mass), use this module's output as input to the Block.
 *
 *  This module defines a 'jacobian' factor, important for the normalisation of the likelihood. Formally, the NWA is defined by
 *  replacing, in the matrix element, a propagator by a Diract delta function:
 *  \f[
 *    \int \! ds \, \left| \mathcal{M} \right|^2 = \int \! ds \, \frac{ \left|\mathcal{M}_d \right|^2}{(s-m^2)^2+(m \Gamma)^2} \to \frac{\pi}{m \Gamma} \int \! ds \, \delta(s-m^2) \left|\mathcal{M}_d \right|^2
 *  \f]
 *  where \f$\left| \mathcal{M}_d \right|^2\f$ is the matrix element squared excluding the propagator, and where the factor \f$\pi/(m\Gamma)\f$ is needed because of the normalisation of the Dirac delta:
 *  
 *  \f$\int_{-\infty}^{+\infty} \! ds \, \delta(s) = 1\f$, but \f$ \int_{-\infty}^{+\infty} \! ds \, \frac{1}{(s-m^2)^2+(m \Gamma)^2} = \frac{\pi}{m\Gamma}\f$.
 *
 *  However, in most of the cases, the matrix element used by the user still includes the propagator (ie, what is used is \f$\mathcal{M}\f$, not \f$\mathcal{M}_d\f$). The propagator
 *  evaluated on \f$s=m^2\f$ is just \f$(m\Gamma)^{-2}\f$, so that the normalisation factor becomes \f$\pi m \Gamma\f$.
 *
 *  This module handles both cases, which can be configured through the `propagator_in_me` parameter.
 *
 * ### Integration dimension
 *
 * This module requires **0** phase-space point.
 *
 * ### Parameters
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `mass` | double | Mass of the propagator one wishes to fix. |
 *   | `width` | double | Width of the corresponding particle. |
 *   | `propagator_in_me` | bool, default: `true` | Whether the propagator is included in the matrix element or not. |
 *
 * ### Inputs
 *
 * None
 *
 * ### Outputs
 *
 *   | Name | Type | %Description |
 *   |------|------|--------------|
 *   | `s` | double | Just \f$s=\text{mass}^2\f$, to be passed to a Block. |
 *   | `jacobian` | double | Overall factor for proper normalisation. |
 *
 * \ingroup modules
 */

#include <momemta/ParameterSet.h>
#include <momemta/Module.h>

class NarrowWidthApproximation: public Module {
    public:

        NarrowWidthApproximation(PoolPtr pool, const ParameterSet& parameters): Module(pool, parameters.getModuleName())
        {
            double mass = parameters.get<double>("mass");
            double width = parameters.get<double>("width");

            *jacobian = M_PI;
            if(parameters.get<bool>("propagator_in_me", true))
                *jacobian *= mass*width; 
            else
                *jacobian /= mass*width; 
            
            *s = mass*mass; 
        }

    private:
        
        std::shared_ptr<double> s = produce<double>("s");
        std::shared_ptr<double> jacobian = produce<double>("jacobian");
};
REGISTER_MODULE(NarrowWidthApproximation);
