@echo off

if not defined DevEnvDir (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >NUL
)

IF NOT EXIST build mkdir build
pushd build

set files=../code/main.c
set compile_flags=/std:c11 /TC /nologo /GR- /EHa- /Oi /WX /W4 /wd4100 /wd4189 /external:anglebrackets /external:W0 /FC /I..\third_party /DCL_TARGET_OPENCL_VERSION=200
set linker_flags=/opt:ref /subsystem:console /LIBPATH:..\lib user32.lib gdi32.lib shell32.lib k4a.lib OpenCL.lib glfw3_mt.lib opengl32.lib

echo Building...
echo:

echo Debug
cl %compile_flags% /MT /Od /Z7 /DDEBUG /D_DEBUG /DUNICODE /Fedebug %files% /link %linker_flags%

echo:

echo Release
cl %compile_flags% /MT /O2 /DDEBUG /DRELEASE /DNDEBUG /DUNICODE /Ferelease %files% /link %linker_flags%

popd
