#!/bin/bash

mkdir -p build
pushd build >/dev/null 2>&1

printf "Building...\n\n"

printf "Debug\n\n"

gcc -o debug_linux ../code/main.c -DDEBUG -D_DEBUG -DCL_TARGET_OPENCL_VERSION=200 -I../third_party -lGL -lOpenCL -lm -lrt -lglfw -pthread -Wno-incompatible-pointer-types 

printf "Release\n\n"

gcc -o release_linux ../code/main.c -O3 -g0 -s -DRELEASE -DNDEBUG -DCL_TARGET_OPENCL_VERSION=200 -I../third_party -lGL -lOpenCL -lm -lrt -lglfw -pthread -Wno-incompatible-pointer-types

popd >/dev/null 2>&1
