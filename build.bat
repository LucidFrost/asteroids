@echo off

where cl >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Importing compiler variables...
    call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
)

set exe_name=asteroids.exe

if not exist build mkdir build
pushd build
    echo Starting building...
    ..\tools\ctime -begin asteroids.ctm

    cl /nologo /Zi /Fe%exe_name% ../src/main.cpp
    set build_result=%ERRORLEVEL%

    if %build_result% equ 0 (
        echo out: %exe_name%
    ) else (
        echo Build failed
    )

    ..\tools\ctime -end asteroids.ctm %build_result%
popd