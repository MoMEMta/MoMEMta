#! /bin/bash

mkdir -p external/root
cd external/root

wget http://cp3.irmp.ucl.ac.be/~delaere/MoMEMta/ROOT-${ROOT_VERSION}_Python-2.7_gcc-4.9.3_Ubuntu_14.04_x86_64.tar.xz

tar xf ROOT-${ROOT_VERSION}_Python-2.7_gcc-4.9.3_Ubuntu_14.04_x86_64.tar.xz
cd ROOT-${ROOT_VERSION}_Python-2.7_gcc-4.9.3_Ubuntu_14.04_x86_64

source bin/thisroot.sh

cd ../../..
