# MoMEMta
The Modular Matrix Element Method implementation.

## Install

### Prerequisites

MoMEMta depends on the following libraries and tools:
   * LHAPDF (>=6)
   * CMake (>=2.8.8)
   * Boost
   * ROOT
   * A C++11-capable compiler

**Note**: MoMEMta has only been tested on GNU/Linux.

### Procedure

Retrieve the code [here](https://github.com/MoMEMta/MoMEMta/archive/prototype.zip) or using `git clone git@github.com:MoMEMta/MoMEMta.git`. Unpack the
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
   * `-DCMAKE_INSTALL_PREFIX=(path)`: Install MoMEMta in a specific location
   * `-DPROFILING=ON`: Generate debugging symbols and profiling information (requires `gperftools`)
   * `-DBOOST_ROOT=(path)`: Use specific Boost library installation
   * `-DTESTS=ON`: Also compile the test executables
   * `-DEXAMPLES=OFF`: Do not compile the example executables

## Examples

After building according to the above instructions, you can try to run the examples:
```
$ ./tt_fullyleptonic.exe
$ ./tt_fullyleptonic_NWA.exe
```
Have a look at the corresponding source and config files are located in `MoMEMta/examples`!

## Documentation

The documentation is available [here](https://momemta.github.io/MoMEMta/).

Content:
- @subpage modules "List of all available modules"
