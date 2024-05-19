@echo off

if not defined DevEnvDir (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >NUL
)

IF NOT EXIST build mkdir build
pushd build

set files=../code/main.c
set compile_flags=/std:c11 /nologo /GR- /EHa- /Oi /WX /W4 /wd4100 /wd4189 /external:anglebrackets /external:W0 /FC /I..\third_party\windows
set linker_flags=/opt:ref /subsystem:console user32.lib gdi32.lib shell32.lib opengl32.lib ..\lib\k4a.lib ..\lib\glfw3_mt.lib

REM Debug Build:
echo Debug Build
cl %compile_flags% /MT /Od /Z7 /DDEBUG /Fe"debug" %files% /link %linker_flags%

REM Release Build
echo Release Build
cl %compile_flags% /MT /O2 /DRELEASE /Fe"release" %files% /link %linker_flags%

popd
