#! /usr/bin/env bash

# Install lcov
mkdir -p coverage
cd coverage

wget https://github.com/linux-test-project/lcov/releases/download/v1.13/lcov-1.13.tar.gz &> /dev/null
tar xf "lcov-1.13.tar.gz"
cd "lcov-1.13"

make PREFIX=$PWD/../lcov install &> /dev/null

PATH="$PWD/../lcov/bin:$PATH"

cd ../..

ROOT='..'
OUTPUT='lcov.info'
GCOV_PATH=$(which $GCOV)

echo "Generating code coverage..."
lcov -q --gcov-tool "$GCOV_PATH" --directory $ROOT --base-directory $ROOT --capture --output-file $OUTPUT

# Exclude some files
EXCLUSIONS=('/usr/*' '*CMakeCXXCompilerId.cpp' '*CMakeCCompilerId.c' '*feature_tests.c*' '*MatrixElements/*' '*/tests/*' '*/MoMEMta/build/*' '*/external/*')

for E in "${EXCLUSIONS[@]}"; do
    lcov -q --gcov-tool "$GCOV_PATH" --remove $OUTPUT "$E" --output-file $OUTPUT
done

lcov --list $OUTPUT
