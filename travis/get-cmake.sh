#! /bin/bash

BUNDLE=cmake-${CMAKE_VERSION}_gcc-4.9.3_Ubuntu_14.04_x86_64

mkdir -p external/cmake
cd external/cmake

wget http://sbrochet.web.cern.ch/sbrochet/public/${BUNDLE}.tar.xz

tar xf ${BUNDLE}.tar.xz
cd ${BUNDLE}

export PATH="$PWD/bin:$PATH"
export LD_LIBRARY_PATH="$PWD/lib:$LB_LIBRARY_PATH"
export _CMAKE_INSTALL_DIR="$PWD"

cd ../../..
