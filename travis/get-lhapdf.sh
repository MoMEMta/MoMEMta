#! /bin/bash

LHAPDF_VERSION=6.1.6
BUNDLE=LHAPDF-${LHAPDF_VERSION}_Python-2.7_gcc-4.9.3_Ubuntu_14.04_x86_64

mkdir -p external/lhapdf
cd external/lhapdf

wget http://sbrochet.web.cern.ch/sbrochet/public/${BUNDLE}.tar.xz

tar xf ${BUNDLE}.tar.xz
cd ${BUNDLE}

# Fix 'lhapdf-config' which hardcode path
sed -i "s#/vagrant/builds/LHAPDF-6.1.6_Python-2.7_gcc-4.9.3_Ubuntu_14.04_x86_64#$PWD#" bin/lhapdf-config
sed -i "s#/vagrant/builds/LHAPDF-6.1.6_Python-2.7_gcc-4.9.3_Ubuntu_14.04_x86_64#$PWD#" bin/lhapdf

export PATH="$PWD/bin:$PATH"
export LD_LIBRARY_PATH="$PWD/lib:$LB_LIBRARY_PATH"
export PYTHONPATH="$PWD/lib/python2.7/site-packages:$PYTHONPATH"

# Install CT10nlo

pushd share/LHAPDF

wget http://www.hepforge.org/archive/lhapdf/pdfsets/6.1/CT10nlo.tar.gz
tar xf CT10nlo.tar.gz

popd

export LHAPDF_DATA_PATH="$PWD/share/LHAPDF"

cd ../../..
