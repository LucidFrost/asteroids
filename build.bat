@echo off

where cl >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Importing compiler variables...
    call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
)

if not exist build mkdir build
pushd build
    echo Building 'asteroids.exe'...
    cl /nologo /Zi ../src/main.cpp /link /out:"asteroids.exe"
popd