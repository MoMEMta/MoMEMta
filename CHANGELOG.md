# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/) 
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]
### Added
 - New `Looper` module to loop over a set of solutions (see below for more details)
 - Python bindings. To enable, pass `-DPYTHON_BINDINGS=ON` to `cmake`.
 - Modules to only evaluate transfer functions (without integrating over anything): BinnedTransferFunctionOnEnergyEvaluator, GaussianTransferFunctionOnEnergyEvaluator
 - Matrix element parameters can now be edited from the configuration file.
 - `ConfigurationReader` constructor now accepts an optional second argument allowing the definition of variables accessible from the lua configuration. 
 - When an exception occurs while constructing a module, try to give useful information to help debugging.
 - Support for CMSSW environment. Python, boost and LHAPDF libraries from CMSSW are automatically used and running `make install` will automatically register MoMEMta as a new external tool.
 - Support use of other integration algorithms implemented in Cuba (Suave, Divonne, Cuhre)
 - Binned and Gaussian transfer function on Pt
 - `DEBUG_TIMING` cmake option, to print a summary of each module runtime at the end of the integration.
 - New cuba option to configure multi-core integration, `ncores` and `pcores`.
 - `declare_input` lua function, used to declare a new input.
 - `append` and `copy_and_append` lua functions are now built-in MoMEMta.
 - Two new lua functions, `add_reco_permutations` and `add_gen_permutations` are available to easily insert a permutator module permutating between the function arguments. 
 - `LinearCombinator` templated module allowing to compute combinations (ie sums, subtractions, ...) of LorentzVectors, numbers, ...
 - Secondary blocks A, B, CD, E
 - Main blocks A, E and G (not present in MadWeight)

### Changed
 - The way to handle multiple solutions coming from blocks has changed. A module is no longer responsible for looping over the solutions itself, this role is delegated to the `Looper` module. As a consequence, most of the module were rewritten to handle this change. See this [pull request](https://github.com/MoMEMta/MoMEMta/pull/69) and [this one](https://github.com/MoMEMta/MoMEMta/pull/91) for a more technical description, and this [documentation entry](http://momemta.github.io/) for more details
 - The way input tags corresponding to phase-space points are defined in Lua has changed. It is no longer allowed to specify them explicitly (through `cuba::ps_point/i`). The function `getpspoint()` has been changed to `add_dimension()` and should be used each time an additional dimension is needed for the integration (the function returns an input tag for the corresponding phase-space components). Modules do not declare how many dimensions they need (counting the number of dimensions is done automatically through the `add_dimension()` function). See this [Pull Request](https://github.com/MoMEMta/MoMEMta/pull/82) for more details.
 - GaussianTransferFunction renamed to GaussianTransferFunctionOnEnergy to be consistent. Also, the width of the Gaussian is computed from the 'gen' energy, not the 'reco' (more correct). The width over which the integration is performed, however, is still computed using the 'reco' energy (no choice there): i.e. integrating over +/- 5 sigma with a resolution of 10% gives an integration range of 'E_gen' in +/- 5\*0.1\*E_reco.
 - Cuba `smoothing` option is now enabled by default to mitigate numerical instabilities during the integration. See [this](https://github.com/MoMEMta/MoMEMta/pull/91) for more details.
 - If no integration dimension is requested, no integration is performed and the graph is only evaluated once. As such, no error is provided on the final result.
 - Cuba logging is now handled by MoMEMta logger, at `debug` level.
 - We no longer use boost-log for logging, but our own implementation heavily inspired by [spdlog](https://github.com/gabime/spdlog). As a consequence, boost-log is no longer required to build MoMEMta.
 - Boost is no longer a dependency when **using** MoMEMta (but it's still a build dependency)
 - `MoMEMta::computeWeights` now expects a vector of `Particle` and no longer a vector of `LorentzVector`. A `Particle` has a name, a `LorentzVector` and a type. As a result, configuration files must now declare which inputs are expected.
 - The way the inputs are passed to the blocks is changed (the particles entering the change of variables are set explicitly, the others are put into the `branches` vector of input tags)
 - Built-in lua version is now v5.3.4

### Fixed
 - Cuba forking mode was broken when building in release mode (with `-DCMAKE_RELEASE_TYPE=Release`).
 - SLHA card reader (matrix element parameter cards) retrieved from MadGraph was broken.
 - Transfer functions on energy take as lower bound the mass of the given "reco" particle
