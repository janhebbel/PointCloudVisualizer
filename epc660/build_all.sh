#!/bin/bash

echo CPU-plus-OpenGL
cd CPU-plus-OpenGL
source ./build.sh
printf "\n"

echo OpenCL
cd ../OpenCL
source ./build.sh
printf "\n"

echo OpenGL
cd ../OpenGL
source ./build.sh
printf "\n"

echo PCL
cd ../PCL
source ./build.sh
printf "\n"

cd ..
