#! /bin/bash

doxygen_VERSION=1.8.13
BUNDLE=doxygen-${doxygen_VERSION}_Python-2.7_gcc-4.9_Ubuntu_14.04_x86_64

mkdir -p external/doxygen
cd external/doxygen

wget http://cp3.irmp.ucl.ac.be/~delaere/MoMEMta/${BUNDLE}.tar.xz

tar xf ${BUNDLE}.tar.xz
cd ${BUNDLE}

export PATH="$PWD/bin:$PATH"
export LD_LIBRARY_PATH="$PWD/lib:$LB_LIBRARY_PATH"

cd ../../..
