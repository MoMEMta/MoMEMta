# MoMEMta

The Modular Matrix Element Method implementation.

[![Build Status](https://travis-ci.org/MoMEMta/MoMEMta.svg?branch=master)](https://travis-ci.org/MoMEMta/MoMEMta) [![Coverage Status](https://coveralls.io/repos/github/MoMEMta/MoMEMta/badge.svg?branch=master)](https://coveralls.io/github/MoMEMta/MoMEMta?branch=master) [![codecov](https://codecov.io/gh/MoMEMta/MoMEMta/branch/master/graph/badge.svg)](https://codecov.io/gh/MoMEMta/MoMEMta)

## Install


### Prerequisites

MoMEMta depends on the following libraries and tools:
   * LHAPDF (>=6)
   * CMake (>= 3.4.0)
   * Boost (>=1.54)
   * ROOT (>=6.20.00) (see the build options below about C++ standards)
   * A C++11-capable compiler

**Note**: MoMEMta has only been tested on GNU/Linux.

### Procedure

Retrieve the code on [our github repository](https://github.com/MoMEMta/MoMEMta/releases). Unpack the
archive and/or go to the `MoMEMta` directory. Next, execute the following:
```
mkdir build
cd build
cmake ..
make -j 4
```
You can now use the library `libmomemta.so` with your own code.

Finally, you can make MoMEMta (public headers and library) available system-wide using:
```
make install
```
This step is **mandatory** if you want to link your own code or matrix elements with MoMEMta. If you don't have
admin rights on your system, have a look at the build options below.

### Build options

The following options are available when configuring the build (when running `cmake ..`):
   * `-DCMAKE_INSTALL_PREFIX=(path)`: Install MoMEMta in a specific location when running `make install` (useful if you don't have admin rights)
   * `-DPROFILING=ON`: Generate debugging symbols and profiling information (requires `gperftools`)
   * `-DBOOST_ROOT=(path)`: Use specific Boost version (path to its install directory)
   * `-DTESTS=ON`: Also compile the test executables
   * `-DEXAMPLES=OFF`: Do not compile the example executables
   * `-DPYTHON_BINDINGS=ON|OFF` (`OFF` by default). Builds python bindings for MoMEMta. Requires python and boost::python. For python3, see notes below.
   * `-DDEBUG_TIMING=ON|OFF` (`OFF` by default). If `ON`, a summary of how long each module ran is printed at the end of the integration. Can be useful to see which module to optimize.
   * `-DCMAKE_CXX_STANDARD=X`, where `X` should be the same version (e.g. `11`, `14`, `17`) as the one used to build the ROOT library.
      - The value of `CMAKE_CXX_STANDARD` used to build ROOT can be found from querying the `root-config`:
         ```
         -DCMAKE_CXX_STANDARD=$(root-config --cflags | sed --regexp-extended 's/.*-std=c\+\+([0-9]+).*/\1/g')
         ```

## Examples

After building according to the above instructions, you can try to run the examples:
```
./tt_fullyleptonic.exe
./tt_fullyleptonic_NWA.exe
```
Have a look at the corresponding source and config files located in `MoMEMta/examples`! 

We also have a [dedicated repository](https://github.com/MoMEMta/Tutorials) with more involved working examples and tutorials.

## Documentation

You can find more details about MoMEMta on our documentations:

 - [The main documentation](https://momemta.github.io)
 - [The technical documentation](https://momemta.github.io/MoMEMta/)

## Python bindings and python3

We support both python2 and python3, but on systems where both versions are installed simultaneously, the build procedure requires a tad more effort.
The following options need to be used when configuring CMake:
   * `-DPYTHON_MIN_VERSION=(ver)`: Force use of python version (e.g. set `(ver)=3` to make sure python3 is used).
   * `-DBoost_PYTHON_VERSION_TAG=(tag)`: The naming of the boost::python library for python3 is inconsistent across systems, and cannot be determined by CMake; you'll have to find its name yourself. The python2 version is named `libboost_python.so`, and for python3 a suffix is added: e.g. `libboost_python3.so` or `libboost_python-py36.so`. The full suffix has to be passed through this option: e.g. `(tag)=3` or `(tag)=-py36`.
Note that CMake might print out a warning of the type "skipping header check" - you can ignore that.
