# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/) 
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]
### Added
 - New `Looper` module to loop over a set of solutions (see below for more details)
 - Secondary block C/D

### Changed
 - The way to handle multiple solutions coming from blocks has changed. A module is no longer responsible for looping over the solutions itself, this role is delegate to the `Looper` module. As a consequence, most of the module were rewritten to handle this change. See this [Pull Request](https://github.com/MoMEMta/MoMEMta/pull/69) for a more technical description, and this [documentation entry](http://momemta.github.io/) for more details
 - The way input tags corresponding to phase-space points are defined in Lua has changed. It is no longer allowed to specify them explicitly (through 'cuba::ps_point/i'). The function 'getpspoint()' has been changed to 'add_dimension()' and should be used each time an additional dimension is needed for the integration (the function returns an input tag for the corresponding phase-space components). Modules do not declare how many dimensions they need (counting the number of dimensions is done automatically through the 'add_dimension()' function). See this [Pull Request](https://github.com/MoMEMta/MoMEMta/pull/82) for more details.
