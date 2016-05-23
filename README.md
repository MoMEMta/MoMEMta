# MoMEMta
The Modular Matrix Element Method implementation.

## Install

### Prerequisites

MoMEMta depends on the following libraries and tools:
   * LHAPDF (>=6)
   * CMake (>=3.2)
   * Boost (>=1.54)
   * ROOT (>=5.34.09)
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

If you have admin rights on your system, you can make MoMEMta (public headers and library) available system-wide using:
```
make install
```

### Build options

The following options are available when configuring the build (when running `cmake ..`):
   * `-DCMAKE_INSTALL_PREFIX=(path)`: Install MoMEMta in a specific location when running `make install` (useful if you don't have admin rights)
   * `-DPROFILING=ON`: Generate debugging symbols and profiling information (requires `gperftools`)
   * `-DBOOST_ROOT=(path)`: Use specific Boost library installation
   * `-DTESTS=ON`: Also compile the test executables
   * `-DEXAMPLES=OFF`: Do not compile the example executables

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
