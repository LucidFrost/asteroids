@echo off

where cl >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Importing compiler variables
    call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
)

set is_debug=1

if "%1" equ "debug"   (set is_debug=1)
if "%1" equ "release" (set is_debug=0)

if %is_debug% equ 1 (
    echo Starting debug build

    set c_flags=/D"DEBUG" /MTd /Od /Zi
    set l_flags=
) else (
    echo Starting release build

    set c_flags=/D"RELEASE" /MT /Ox
    set l_flags=
)

if not exist build mkdir build
pushd build
    ..\tools\ctime -begin asteroids.ctm

    cl /nologo /Fe"asteroids.exe" /D"OS_WINDOWS" /EHa- /WX /W4 %c_flags% ../src/main.cpp /link %l_flags%
    set build_result=%ERRORLEVEL%

    ..\tools\ctime -end asteroids.ctm %build_result%
popd

if %build_result% equ 0 (
    if %is_debug% equ 1 (
        REM call run.bat
    ) else (
        echo Packaging release
        
        if exist release (rmdir release /s /q)
        mkdir release

        mkdir release\fonts
        mkdir release\sounds
        mkdir release\sprites

        xcopy "data/fonts"   "release/fonts"   /e
        xcopy "data/sounds"  "release/sounds"  /e
        xcopy "data/sprites" "release/sprites" /e
        
        xcopy "build\asteroids.exe" "release"
    )
)

echo Done