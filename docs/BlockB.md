# Block B description
Note: Block B, also referred as final block (FB), and class in the MadWeight world. 
Description inspired on MadWeight.

This block corresponds to an initial state with Bjorken fractions \f$q_1\f$ and \f$q_2\f$, 
and a final state with a visible particle with momenta \f$p_2\f$, and an invisible particle 
with momenta \f$p_1\f$, both coming from a resonance with mass \f$s_{12}\f$. Extra radiation (ISR) it is also allowed.

The goal of this Block is to address the change of variables needed to pass from the standard phase-space
parametrization to the \f$\frac{1}{4\pi E_1} ds_{12} \times J\f$ parametrization. Per integration point 
in CUBA, this Block outputs the value of the jacobian, J, and the four momenta of the invisible particle, 
\f$p_1\f$. The system of equations needed to compute \f$p_1\f$ is described below. 


### Change of variables

From the standard phase-space parametrisation:

\f$dq_1 dq_2\frac{d^3 p_1}{(2\pi)^3 2E_1}(2\pi)^4 \delta^4 (P_{in}-P_{fin})\f$

where \f$P_{in} and P_{fin)\f$ are the total four-momenta in the initial and final states, to the 
following parametrisation: 

\f$\frac{1}{4\pi E_1} ds_{12} \times J\f$

where the jacobian, J, is given by:

\f$J = \frac{E_1}{s} |p_{2z} E_1 -  E_2 p_{1z}|^{-1}\f$

### Parameters

- Collision energy.

### Inputs

- Visible particle, with four momenta \f$p_2\f$
- The mass \f$s_{12}\f$ as output from the Flatter module.

### Outputs

- Invisible particle, with four momenta \f$p_1\f$ (up to two solutions possible)
- Jacobian, one per solution.    

### System of equations to compute \f$p_1\f$

The integrator throws random points in the mass (\f$s_{12}\f$) while \f$p_2\f$ is a known quantity. The equations to 
compute \f$p_1\f$ are:

 1. \f$(p_1 + p_2)^2 = s_{12} = M_{1}^{2} + M_{2}^2 + 2 E_1 E_2 + 2 p_{1x}p_{2x} + 2p_{1y}p_{2y} + p_{1z}p_{2z}\f$
 2. \f$p_{1x} = - p_{Tx}\f$ #Coming from \f$p_{T}^{\nu} = -p_{T}^{visible} = - (p_2 + ISR)\f$
 3. \f$p_{1y} = - p_{Ty}\f$ #Coming from \f$p_{T}^{\nu} = -p_{T}^{visible} = - (p_1 + ISR)\f$
 4. \f$M_1 = 0 \to E_{1}^2 = p_{1x}^2 + p_{1y}^2 + p_{1z}^2\f$

Using the values of \f$p_{1x}, p_{1y}\f$ from equations (2) and (3), equation (1) can be written as \f$p_{1z} = A - B E_1\f$. Where A and B are:

- \f$A = \frac{s_{12} - M_{2}^2 + 2(p_{Tx}p_{2x} + p_{Ty}p_{2y})}{2 p_{2z}}\f$
- \f$B = \frac{E_2}{p_{2z}}\f$

Finally equation (4) can be written as: 

- \f$(1 - B) E_{1}^2 + 2AB E_1 - C = 0\f$, where \f$C = p_{Tx}^{2} + p_{Ty}^{2}\f$

Each solution of the quadratic equation with a positive value of $$E_1\f$ is taken.

