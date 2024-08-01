@echo off
setlocal
pushd "%CD%"

set OPT=%1

if [%OPT%] equ [] goto Block_Make
if [%OPT%] equ [clean] goto Block_Clean

:Block_Clean
del /f /q sabtr.exe >nul 2>&1
goto Finish

:Block_Make
del /f /q sabtr.exe >nul 2>&1
tcc sabtr.c -lshell32 -ladvapi32 -o sabtr.exe
goto Finish

:Finish
popd
endlocal
