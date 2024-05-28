@echo off

cd AzureKinect
call build_all.bat
cd ..\epc660
call build_all.bat
cd ..
