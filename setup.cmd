@echo off

REM Check if the vcpkg path exists
if not exist "%VCPKG_ROOT%" (
    echo Error: VCPKG_ROOT directory does not exist: %VCPKG_ROOT%
    exit /b 1
)

REM Run the CMake command with the specified toolchain file and target triplet
cmake -B .build -S . -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows

REM Check if CMake succeeded
if errorlevel 1 (
    echo Error: CMake configuration failed.
    exit /b 1
)

echo CMake configuration succeeded.
