#!/bin/bash

set -e
set -x

rm -rf build
mkdir build
pushd build

conan install ..
cmake --trace .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

