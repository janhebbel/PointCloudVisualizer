@echo off

cd build
call msbuild PCL.sln /p:Configuration=Debug
call msbuild PCL.sln /p:Configuration=Release
cd ..
