@echo off

IF NOT EXIST build mkdir build
pushd build

set files=../code/main.c
set compile_flags=/std:c11 /nologo /GR- /EHa- /Oi /WX /W4 /wd4100 /wd4189 /external:anglebrackets /external:W0 /FC /I..\third_party
set linker_flags=/opt:ref /subsystem:console user32.lib gdi32.lib shell32.lib ..\lib\k4a.lib

echo Building...
echo:

echo Debug
cl %compile_flags% /MT /Od /Z7 /DDEBUG /D_DEBUG /DUNICODE /Fe"debug" %files% /link %linker_flags%

echo:

echo Release
cl %compile_flags% /MT /O2 /DRELEASE /DNDEBUG /DUNICODE /Fe"release" %files% /link %linker_flags%

popd
