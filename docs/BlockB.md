# Block B description
Note: Block B, also refered as final block (FB), and class in the MadWeight world. Description inspired on the Olivier Mattelaer's PhD [thesis]. 
[thesis]: <https:FIXME>

This block corresponds to an initial state with Bjorken fractions $$q_1$$ and $$q_2$$, and a final state with a visible particle with momenta $$p_2$$, and an invisible particule with momenta $$p_1$$, both coming from a resonace with mass $$s_{12}$$. Extra radiation (ISR) it is also allowed.

### Change of variables

From 

$$dq_1 dq_2\frac{d^3 p_1}{(2\pi)^3 2E_1}(2\pi)^4 \delta^4 (P_{in}-P_{fin})$$

to 

$$\frac{1}{4\pi E_1} ds_{12} \times J$$

where J is given by:

$$J = \frac{E_1}{s} |p_{2z} E_1 -  E_2 p_{1z}|^{-1}$$

### Inputs

- Visible particle, with four momenta $$p_2$$
- The mass $$s_{12}$$ as output from the Flatter

### Outputs

- Jacobian
- Invisible particle, with four momenta $$p_1$$ (more than one value if several solutions are )
    

### Integration flow

The integrator through random points in the mass ($$s_{12}$$) while $$p_2$$ is a known quantity. The equations to 
solve $$p_1$$ are:

- (1) $$(p_1 + p_2)^2 = s_{12} = M_{1}^{2} + M_{2}^2 + 2 E_1 E_2 + 2 p_{1x}p_{2x} + 2p_{1y}p_{2y} + p_{1z}p_{2z}$$
- (2) $$p_{1x} = - p_{Tx}$$ #Coming from $$p_{T}^{\nu} = -p_{T}^{visible} = - (p_2 + ISR)$$
- (3) $$p_{1y} = - p_{Ty}$$ #Coming from $$p_{T}^{\nu} = -p_{T}^{visible} = - (p_1 + ISR)$$
- (4) $$M_1 = 0 \to E_{1}^2 = p_{1x}^2 + p_{1y}^2 + p_{1z}^2$$

Using the values of $$p_{1x}, p_{1y}$$ from equations (2) and (3), equation (1) can be written as $$p_{1z} = A - B E_1$$. Where A and B are:

- $$A = \frac{s_{12} - M_{2}^2 + 2(p_{Tx}p_{2x} + p_{Ty}p_{2y})}{2 p_{2z}}$$
- $$B = \frac{E_2}{p_{2z}}$$

Finally equation (4) can be written as: 

- $$(1 - B) E_{1}^2 + 2AB E_1 - C = 0$$, where $$C = p_{Tx}^{2} + p_{Ty}^{2}$$

Each solution of the quadratic equation with a positive value of $$E_1$$ is taken.