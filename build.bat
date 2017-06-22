@echo off

where cl >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Importing compiler variables...
    call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
)

if not exist build mkdir build
pushd build
    ..\tools\ctime -begin asteroids.ctm

    cl /nologo /Zi /Fe"asteroids.exe" ../src/main.cpp
    set build_result=%ERRORLEVEL%

    ..\tools\ctime -end asteroids.ctm %build_result%
popd