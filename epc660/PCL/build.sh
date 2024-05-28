#!/bin/bash

mkdir -p build_linux
pushd build_linux >/dev/null 2>&1

printf "Building...\n\n"

cmake -DCMAKE_BUILD_TYPE=Release ..
make

popd >/dev/null 2>&1
