# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/) 
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]
### Added
 - New `Looper` module to loop over a set of solutions (see below for more details)

### Changed
 - The way to handle multiple solutions coming from blocks has changed. A module is no longer responsible for looping over the solutions itself, this role is delegate to the `Looper` module. As a consequence, most of the module were rewritten to handle this change. See this [Pull Request](https://github.com/MoMEMta/MoMEMta/pull/69) for a more technical description, and this [documentation entry](http://momemta.github.io/) for more details
