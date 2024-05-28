@echo off

echo CPU-based
cd CPU-based
call build.bat
echo:

echo CPU-plus-OpenGL
cd ..\CPU-plus-OpenGL
call build.bat
echo:

echo OpenCL
cd ..\OpenCL
call build.bat
echo:

echo OpenGL
cd ..\OpenGL
call build.bat
echo:

echo PCL
cd ..\PCL
call build.bat
echo:

cd ..
