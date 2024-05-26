#!/bin/bash

mkdir -p build
pushd build >/dev/null 2>&1

gcc -o debug_linux ../code/main.c -I../third_party -lGL -lm -lrt -lglfw -pthread -Wno-incompatible-pointer-types 
# -ldl

popd >/dev/null 2>&1
