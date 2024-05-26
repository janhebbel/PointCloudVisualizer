#!/bin/bash

mkdir -p build
pushd build >/dev/null 2>&1

printf "Building...\n\n"

printf "Debug\n\n"

gcc -o debug_linux ../code/main.c -DDEBUG -D_DEBUG -I../third_party -lGL -lm -lrt -lglfw -pthread -Wno-incompatible-pointer-types 

printf "Release\n\n"

gcc -o release_linux ../code/main.c -O3 -g0 -s -DRELEASE -DNDEBUG -I../third_party -lGL -lm -lrt -lglfw -pthread -Wno-incompatible-pointer-types

popd >/dev/null 2>&1
