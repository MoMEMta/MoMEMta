#! /bin/bash

mkdir -p external/lhapdf
cd external/lhapdf

wget http://www.hepforge.org/archive/lhapdf/LHAPDF-6.1.6.tar.gz
tar xf LHAPDF-6.1.6.tar.gz
cd LHAPDF-6.1.6

./configure --prefix=$PWD/..
make install -j2

cd ..

export PATH="$PWD/bin:$PATH"
export LD_LIBRARY_PATH="$PWD/lib:$LB_LIBRARY_PATH"

cd ../..
