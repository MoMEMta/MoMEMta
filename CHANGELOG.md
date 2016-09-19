# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/) 
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]
### Added
 - New `Looper` module to loop over a set of solutions (see below for more details)
 - Secondary block C/D
 - Python bindings. To enable, pass `-DPYTHON_BINDINGS=ON` to `cmake`.
 - Main block A
 - Modules to only evaluate transfer functions (without integrating over anything): BinnedTransferFunctionOnEnergyEvaluator, GaussianTransferFunctionOnEnergyEvaluator
 - Matrix element parameters can now be edited from the configuration file.

### Changed
 - The way to handle multiple solutions coming from blocks has changed. A module is no longer responsible for looping over the solutions itself, this role is delegated to the `Looper` module. As a consequence, most of the module were rewritten to handle this change. See this [pull request](https://github.com/MoMEMta/MoMEMta/pull/69) and [this one](https://github.com/MoMEMta/MoMEMta/pull/91) for a more technical description, and this [documentation entry](http://momemta.github.io/) for more details
 - The way input tags corresponding to phase-space points are defined in Lua has changed. It is no longer allowed to specify them explicitly (through `cuba::ps_point/i`). The function `getpspoint()` has been changed to `add_dimension()` and should be used each time an additional dimension is needed for the integration (the function returns an input tag for the corresponding phase-space components). Modules do not declare how many dimensions they need (counting the number of dimensions is done automatically through the `add_dimension()` function). See this [Pull Request](https://github.com/MoMEMta/MoMEMta/pull/82) for more details.
 - GaussianTransferFunction renamed to GaussianTransferFunctionOnEnergy to be consistent. Also, the width of the Gaussian is computed from the 'gen' energy, not the 'reco' (more correct). The width over which the integration is performed, however, is still computed using the 'reco' energy (no choice there): i.e. integrating over +/- 5 sigma with a resolution of 10% gives an integration range of 'E_gen' in +/- 5*0.1*E_reco.
 - Cuba `smoothing` option is now enabled by default to mitigate numerical instabilities during the integration. See [this](https://github.com/MoMEMta/MoMEMta/pull/91) for more details.
